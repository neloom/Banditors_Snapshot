// Copyright (C) 2024 by Haguk Kim


#include "TRPhysicsHandleComponent.h"

UTRPhysicsHandleComponent::UTRPhysicsHandleComponent()
{
	SetIsReplicatedByDefault(false); // 피직스 핸들 액션의 결과만 Replicate
}
