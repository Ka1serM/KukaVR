#include "LevelDataAsset.h"

int32 ULevelDataAsset::GetTaskCount() const
{
	return Tasks.Num();
}

bool ULevelDataAsset::HasTasks() const
{
	return Tasks.Num() > 0;
}



bool ULevelDataAsset::HasLevelSpawnedActors() const {
	return LevelSpawnedActors.Num() > 0;
}
