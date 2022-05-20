//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Tasks", description: "Move task.", color: "0 0 255 255")]
class SCR_EliminateTaskClass: SCR_BaseTaskClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_EliminateTask : SCR_BaseTask
{
	[Attribute()]
	protected float m_fRange;
	
	protected RplId m_TargetEntityRplID;
	protected IEntity m_TargetEntity;

	//------------------------------------------------------------------------------------------------
	RplId GetTargetEntityRplID()
	{
		return m_TargetEntityRplID;
	}

	//------------------------------------------------------------------------------------------------
	override void Finish(bool showMsg = true)
	{
		super.Finish(showMsg);
		
		if (!showMsg)
			return;
		
		SCR_PopUpNotification.GetInstance().PopupMsg("Target eliminated!"); // Never returns null
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_TargetEntity)
			SetOrigin(m_TargetEntity.GetOrigin());
		else
		{
			ClearEventMask(EntityEvent.FRAME);
			ClearFlags(EntityFlags.ACTIVE, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetTargetEntity(notnull IEntity targetEntity)
	{
		RplComponent rplComponent = RplComponent.Cast(targetEntity.FindComponent(RplComponent));
		if (!rplComponent)
			return; // Not replicated, cannot be target

		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(targetEntity.FindComponent(EventHandlerManagerComponent));
		if (!eventHandlerManager)
			return; // No event handler, cannot be target

		m_TargetEntityRplID = rplComponent.Id(); // Store the target entity RplID for replication
		m_TargetEntity = targetEntity;

		eventHandlerManager.RegisterScriptHandler("OnDestroyed", targetEntity, OnDestroyed);

		SetEventMask(EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, false);
	}

	//------------------------------------------------------------------------------------------------
	void OnDestroyed(IEntity targetEntity)
	{
		EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(targetEntity.FindComponent(EventHandlerManagerComponent));
		if (eventHandlerManager)
			eventHandlerManager.RemoveScriptHandler("OnDestroyed", targetEntity, OnDestroyed);

		// State is equal to EDamageState.DESTROYED, finish task
		if (GetTaskManager())
			GetTaskManager().FinishTask(this);
	}

	//------------------------------------------------------------------------------------------------
	override void Serialize(ScriptBitWriter writer)
	{
		super.Serialize(writer);

		writer.WriteRplId(m_TargetEntityRplID);
	}

	//------------------------------------------------------------------------------------------------
	bool QueryEntitiesCallback(IEntity e)
	{
		if (!e.FindComponent(EventHandlerManagerComponent))
			return true;
		
		SetTargetEntity(e);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		// Server only
		if (GetTaskManager() && GetTaskManager().IsProxy())
			return;
		
		// Call next frame, entities may not be initialized
		GetGame().GetCallqueue().CallLater(QueryLater, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	void QueryLater()
	{
		// Attaches to nearest suitable entity within m_fRange
		GetWorld().QueryEntitiesBySphere(GetOrigin(), m_fRange, QueryEntitiesCallback);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_EliminateTask(IEntitySource src, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_EliminateTask()
	{
	}

};