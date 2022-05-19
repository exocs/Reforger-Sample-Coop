[EntityEditorProps(category: "GameScripted/Coop", description: "Allows spawning of AI groups.")]
class SCR_AISpawnerComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_AISpawnerComponent : ScriptComponent
{
	[Attribute("{5BEA04939D148B1D}Prefabs/Groups/INDFOR/Group_FIA_FireTeam.et", UIWidgets.EditBox, "Group prefab to spawn.")]
	protected ResourceName m_rnDefaultPrefab;

	[Attribute("0", UIWidgets.CheckBox, "If checked, spawns group immediately.")]
	protected bool m_bSpawnImmediately;

	[Attribute("0", UIWidgets.CheckBox, "If checked tries to find parent trigger to hook onto.")]
	protected bool m_bUseTrigger;
	
	[Attribute("", UIWidgets.EditBox, "World position for the group to spawn at.", params: "inf inf 0 0 purposeCoords spaceWorld")]
	protected vector m_vSpawnPosition;
	
	[Attribute("", UIWidgets.EditBox, "World rotation for the group to spawn with (only yaw is taken into account).", params: "inf inf 0 0 purposeAngles spaceWorld")]
	protected vector m_vSpawnRotation;

	[Attribute("", UIWidgets.Auto, "List of waypoints to use.")]
	protected ref array<string> m_aWaypointsList;
	
	[Attribute("0", UIWidgets.CheckBox, "Ends game when all AIs die")]
	protected bool m_bEndGameOnEmpty;

	// Attached component.
	protected RplComponent m_pRplComponent;

	//! Spawned agent relevant to the authority only.
	protected AIAgent m_pSpawnedAgent;
	
	//------------------------------------------------------------------------------------------------
	AIAgent GetSpawnedAgent()
	{
		return m_pSpawnedAgent;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetSpawnPosition()
	{
		return m_vSpawnPosition;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetSpawnRotation()
	{
		return Vector(0.0, m_vSpawnRotation[1], 0.0);
	}
	
	//------------------------------------------------------------------------------------------------
	void GetSpawnTransform(out vector transformMatrix[4])
	{
		vector rotation = GetSpawnRotation();
		vector yawPitchRoll = Vector(rotation[1], rotation[0], rotation[2]);
		Math3D.AnglesToMatrix(rotation, transformMatrix);		
		transformMatrix[3] = GetSpawnPosition();
	}

	//------------------------------------------------------------------------------------------------
	bool DoSpawn(ResourceName aiAgentPrefab, array<string> wayPoints = null)
	{
		if (IsSpawned())
		{
			Print("SCR_AISpawnerComponent cannot spawn group; group was spawned already!");
			return false;
		}

		if (!VerifyRplComponentPresent())
		{
			Print("SCR_AISpawnerComponent cannot spawn group, spawner has no RplComponent!");
			return false;
		}

		if (!m_pRplComponent.IsMaster())
		{
			Print("SCR_AISpawnerComponent caught non-master request to spawn!");
			return false;
		}

		Resource agentPrefab = Resource.Load(aiAgentPrefab);
		if (!agentPrefab)
		{
			Print(string.Format("SCR_AISpawnerComponent could not load '%1'", agentPrefab));
			return false;
		}

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;		
		GetSpawnTransform(spawnParams.Transform);

		IEntity spawnedEntity = GetGame().SpawnEntityPrefab(agentPrefab, GetOwner().GetWorld(), spawnParams);
		if (!spawnedEntity)
		{
			Print(string.Format("SCR_AISpawnerComponent could not spawn '%1'", agentPrefab));
			return false;
		}

		AIAgent agent = AIAgent.Cast(spawnedEntity);
		if (!agent)
		{
			Print(string.Format("SCR_AISpawnerComponent spawned entity '%1' that is not of AIAgent type, deleting!", agentPrefab));
			RplComponent.DeleteRplEntity(spawnedEntity, false);
			return false;
		}

		// Store agent
		m_pSpawnedAgent = agent;

		if (m_aWaypointsList && !m_aWaypointsList.IsEmpty())
		{
			BaseWorld world = GetOwner().GetWorld();
			foreach (string waypointName : m_aWaypointsList)
			{
				IEntity waypointEntity = world.FindEntityByName(waypointName);
				if (!waypointEntity)
				{
					Print(string.Format("SCR_AISpawnerComponent could not find waypoint '%1', skipping!", waypointName));
					continue;
				}

				AIWaypoint waypoint = AIWaypoint.Cast(waypointEntity);
				if (!waypoint)
				{
					Print(string.Format("SCR_AISpawnerComponent could not use waypoint '%1': Entity is not AIWaypoint!", waypointName));
					continue;
				}

				agent.AddWaypoint(waypoint);
			}
		}

		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(agent);
		if (aiGroup)
		{
			aiGroup.GetOnEmpty().Insert(OnEmpty);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEmpty()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		Faction faction = GetGame().GetFactionManager().GetFactionByKey("US");
		int usIndex = GetGame().GetFactionManager().GetFactionIndex(faction);
		gameMode.EndGameMode(SCR_GameModeEndData.CreateSimple(SCR_GameModeEndData.ENDREASON_EDITOR_FACTION_VICTORY, -1, usIndex));
	}

	//------------------------------------------------------------------------------------------------
	bool IsSpawned()
	{
		return m_pSpawnedAgent != null;
	}

	//------------------------------------------------------------------------------------------------
	bool DoSpawnDefault()
	{
		return DoSpawn(m_rnDefaultPrefab);
	}
	
#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	protected override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		super._WB_AfterWorldUpdate(owner, timeSlice);
		
		vector spawnPosition = GetSpawnPosition();
		Shape shape = Shape.CreateSphere(COLOR_YELLOW, ShapeFlags.ONCE | ShapeFlags.NOOUTLINE, spawnPosition, 0.3);
		Shape arrow = Shape.CreateArrow(GetOwner().GetOrigin(), spawnPosition, 0.1, COLOR_YELLOW, ShapeFlags.ONCE);
	}
#endif

	//------------------------------------------------------------------------------------------------
	protected bool VerifyRplComponentPresent()
	{
		if (!m_pRplComponent)
		{
			Print("SCR_AISpawnerComponent does not have a RplComponent attached!");
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTriggerActivate()
	{
		/*

		SCR_BaseTriggerEntity parentTrigger = SCR_BaseTriggerEntity.Cast(GetOwner());
		array<IEntity> entitiesInside = {};
		parentTrigger.GetEntitiesInside(entitiesInside);

		Or just inherit (extend) the trigger and get the proper callbacks
		*/

		// Spawn when whatever enters this trigger
		if (!IsSpawned())
		{
			if (DoSpawnDefault())
			{
				// Once the group is spawned, in this case let's just disable the
				// trigger, making it a complete one-shot
				GenericEntity.Cast(GetOwner()).Deactivate();
				// Additionally we could just RplComponent.DeleteEntity(GetOwner(), false);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_pRplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!VerifyRplComponentPresent())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (m_bSpawnImmediately)
		{
			// Spawning of Replicated items must not happen in EOnInit,
			// we delay the call to happen asap (after EOnInit)
			GetGame().GetCallqueue().CallLater(DoSpawnDefault, 0);
		}

		if (m_bUseTrigger)
		{
			SCR_BaseTriggerEntity parentTrigger = SCR_BaseTriggerEntity.Cast(owner);
			if (!parentTrigger)
			{
				Print("SCR_AISpawnerComponent cannot hook to trigger, it is not a child of SCR_BaseTriggerEntity!");
				return;
			}

			parentTrigger.GetOnActivate().Insert(OnTriggerActivate);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (m_bUseTrigger)
		{
			SCR_BaseTriggerEntity parentTrigger = SCR_BaseTriggerEntity.Cast(owner);
			if (!parentTrigger)
				return;

			parentTrigger.GetOnActivate().Remove(OnTriggerActivate);
		}
		
		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(GetSpawnedAgent());
		if (aiGroup)
		{
			aiGroup.GetOnEmpty().Remove(OnEmpty);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SCR_AISpawnerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_AISpawnerComponent()
	{
	}

}
