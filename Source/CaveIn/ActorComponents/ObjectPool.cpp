#include "ObjectPool.h"
#include "CaveIn/Actors/PooledActor.h"
#include "Engine/World.h"

UObjectPool::UObjectPool()
{
	PrimaryComponentTick.bCanEverTick = false;
	PoolSize = 100;
	PooledObjectSubclass = nullptr;
}

void UObjectPool::BeginPlay()
{
	Super::BeginPlay();
	InitPool();
}

void UObjectPool::InitPool() 
{
	if (PooledObjectSubclass != nullptr && GetWorld()) 
	{
		for (int32 i = 0; i < PoolSize; i++) 
		{
			APooledActor* PoolableActor = GetWorld()->SpawnActor<APooledActor>(PooledObjectSubclass, FVector::ZeroVector, FRotator::ZeroRotator);
			PoolableActor->SetActive(false);
			Pool.Add(PoolableActor);
		}
	}
}

// Return the first inactive pooled actor we find
APooledActor* UObjectPool::GetPooledObject() 
{
	for (APooledActor* PooledActor : Pool)
	{
		if (!PooledActor->IsActive()) 
		{
			return PooledActor;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Could not find an inactive pooled actor"));
	return nullptr;
}