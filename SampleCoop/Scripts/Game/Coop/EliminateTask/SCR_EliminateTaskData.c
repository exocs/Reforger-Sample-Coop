//------------------------------------------------------------------------------------------------
class SCR_EliminateTaskData : SCR_BaseTaskData
{
	static const int ELIMINATE_TASK_DATA_SIZE = 4;
	
	protected RplId m_TargetEntityRplID;
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseTaskSupportClass GetSupportClass()
	{
		return GetTaskManager().GetSupportedTaskByTaskType(SCR_EliminateTask);
	}
	
	//------------------------------------------------------------------------------------------------
	override void LoadDataFromTask(SCR_BaseTask task)
	{
		super.LoadDataFromTask(task);
		
		SCR_EliminateTask eliminateTask = SCR_EliminateTask.Cast(task);
		if (!eliminateTask)
			return;
		
		m_TargetEntityRplID = eliminateTask.GetTargetEntityRplID();
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetupTask(SCR_BaseTask task)
	{
		SCR_EliminateTask eliminateTask = SCR_EliminateTask.Cast(task);
		if (!eliminateTask)
			return;
		
		super.SetupTask(eliminateTask);
		
		IEntity targetEntity = IEntity.Cast(Replication.FindItem(m_TargetEntityRplID));
		if (!targetEntity)
			return;
		
		eliminateTask.SetTargetEntity(targetEntity);
	}

	//------------------------------------------------------------------------------------------------
	override void Deserialize(ScriptBitReader reader)
	{
		super.Deserialize(reader);
		
		reader.ReadRplId(m_TargetEntityRplID);
	}
	
	//------------------------------------------------------------------------------------------------
	static override int GetDataSize()
	{
		return ELIMINATE_TASK_DATA_SIZE + SCR_BaseTaskData.GetDataSize();
	}
	
	//################################################################################################
	//! Codec methods
	//------------------------------------------------------------------------------------------------
	static override void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, GetDataSize());
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		return snapshot.Serialize(packet, GetDataSize());
	}
	
	//------------------------------------------------------------------------------------------------
	static override bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx) 
	{
		return lhs.CompareSnapshots(rhs, GetDataSize());
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_EliminateTaskData prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		return snapshot.Compare(prop.m_TargetEntityRplID, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_EliminateTaskData prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		snapshot.SerializeBytes(prop.m_TargetEntityRplID, 4);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_EliminateTaskData prop) 
	{
		snapshot.SerializeBytes(prop.m_TargetEntityRplID, 4);
		
		return true;
	}
}