[EntityEditorProps(category: "GameScripted/Coop", description: "Allows task management.")]
class SCR_CoopTaskManagerClass : SCR_BaseTaskManagerClass
{
}

//------------------------------------------------------------------------------------------------
class SCR_CoopTaskManager : SCR_BaseTaskManager
{
	[Attribute("", UIWidgets.Auto, "Entity names of intial tasks - assigned to connecting players automatically.", category: "TaskManager: COOP")]
	protected ref array<string> m_aInitialTaskNames;

	[Attribute("US", UIWidgets.EditBox, "Faction key to assign to tasks.", category: "TaskManager: COOP")]
	protected FactionKey m_sAssignedFaction;

	//! Runtime instances for tasks created from initial task names
	protected ref array<SCR_BaseTask> m_aInitialTasks;

	protected RplComponent m_pRplComponent;
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_CreateEliminateTask(SCR_EliminateTaskData taskData, array<int> assignees)
	{
		if (!taskData)
			return;
		
		if (GetTask(taskData.GetTaskID()))
			return;
		
		taskData.CreateTask();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		// Do not initialize these tasks out of runtime
		if (!GetGame().InPlayMode())
			return;

		m_aInitialTasks = {};
		BaseWorld world = owner.GetWorld();
		foreach (string taskName : m_aInitialTaskNames)
		{
			IEntity entity = world.FindEntityByName(taskName);
			SCR_BaseTask task = SCR_BaseTask.Cast(entity);
			if (!task)
				continue;
			
			m_aInitialTasks.Insert(task);
		}

		m_pRplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_pRplComponent)
		{
			Print("SCR_CoopTaskManager is missing m_pRplComponent!", LogLevel.ERROR);
			return;
		}

		// Authority only
		if (!m_pRplComponent.IsMaster())
			return;

		Faction targetFaction = GetGame().GetFactionManager().GetFactionByKey(m_sAssignedFaction);
		if (!targetFaction)
		{
			Print("SCR_CoopTaskManager is missing Faction to assign tasks to!", LogLevel.ERROR);
			return;
		}

		foreach (SCR_BaseTask task : m_aInitialTasks)
		{
			if (!task)
				continue;

			task.SetTargetFaction(targetFaction);
		}
		
		// Generating a task ahead!
		// Only on the server
		SCR_EliminateTaskSupportClass eliminateTaskSupportClass = SCR_EliminateTaskSupportClass.Cast(GetSupportedTaskBySupportClassType(SCR_EliminateTaskSupportClass));
		if (!eliminateTaskSupportClass)
			return;
		
		// Code ahead would generate eliminate task, but for now we have it manually placed in map
		/*SCR_EliminateTask eliminateTask = eliminateTaskSupportClass.GenerateNewEliminateTask(world.FindEntityByName("Target_UAZ"));
		if (!eliminateTask)
			return;
		
		eliminateTask.SetTargetFaction(targetFaction);*/
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnPlayerRegistered(int registeredPlayerID)
	{
		super.OnPlayerRegistered(registeredPlayerID);

		SCR_BaseTaskExecutor taskExecutor = SCR_BaseTaskExecutor.GetTaskExecutorByID(registeredPlayerID);
		if (!taskExecutor)
		{
			Print("No execution can be executed because executing execution execute exec.");
			return;
		}
		
		SCR_BaseTask task = m_aInitialTasks.GetRandomElement();
		if (!task)
			continue;
		
		// Skip already finished tasks
		if (task.GetTaskState() == SCR_TaskState.FINISHED)
			continue;

		// Assign task
		AssignTask(task, taskExecutor, false);

		//Example
		/*foreach (SCR_BaseTask task : m_aInitialTasks)
		{
			if (!task)
				continue;

			// Skip already finished tasks
			if (task.GetTaskState() == SCR_TaskState.FINISHED)
				continue;

			// Assign task
			AssignTask(task, taskExecutor, false);
		}*/
	}
}
