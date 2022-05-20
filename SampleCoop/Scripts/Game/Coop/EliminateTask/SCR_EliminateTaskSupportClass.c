//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_EliminateTaskSupportClass : SCR_BaseTaskSupportClass
{
	//------------------------------------------------------------------------------------------------
	//! Call this on server only
	//! This is how it should ideally be done, so the generate methods are not part of the task manager itself
	SCR_EliminateTask GenerateNewEliminateTask(notnull IEntity targetEntity)
	{
		if (!GetTaskManager()) // Don't cache this, it gets in-lined!
			return null; // No task manager!
		
		SCR_EliminateTask eliminateTask = SCR_EliminateTask.Cast(CreateTask());
		if (!eliminateTask)
			return null; // Maybe the prefab/ResourceName was wrong?
		
		eliminateTask.SetTargetEntity(targetEntity);
		
		array<int> assigneeIDs = {};
		array<SCR_BaseTaskExecutor> assignees = {};
		int assigneesCount = eliminateTask.GetAssignees(assignees);
		
		SCR_CampaignTaskData taskData = new SCR_CampaignTaskData();
		taskData.LoadDataFromTask(eliminateTask);
		
		// Now we need specific task manager type, so we cache it
		SCR_CoopTaskManager coopTaskManager = SCR_CoopTaskManager.Cast(GetTaskManager());
		if (coopTaskManager)
			coopTaskManager.Rpc(coopTaskManager.RPC_CreateEliminateTask, taskData, assignees);
		
		return eliminateTask;
	}
	
	//------------------------------------------------------------------------------------------------
	override SCR_EliminateTaskData CreateTaskData()
	{
		return new SCR_EliminateTaskData();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_EliminateTaskSupportClass()
	{
		m_TypeName = SCR_EliminateTask;
	}
}