#include "PooledActor.h"

APooledActor::APooledActor()
{
	PrimaryActorTick.bCanEverTick = false;
	SetActorEnableCollision(false);
}

void APooledActor::BeginPlay()
{
	Super::BeginPlay();
}

bool APooledActor::IsActive() 
{
	return Active;
}

void APooledActor::SetActive(bool NewValue) 
{
	Active = NewValue;
	SetActorHiddenInGame(!Active);
	SetActorEnableCollision(Active);	
}