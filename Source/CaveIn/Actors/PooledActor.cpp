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

void APooledActor::SetActive(bool NewValue) 
{
	Active = NewValue;
	SetActorHiddenInGame(!Active);
	SetActorEnableCollision(Active);	
}

bool APooledActor::IsActive() 
{
	return Active;
}