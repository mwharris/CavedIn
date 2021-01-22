#include "ObjectPool.h"

UObjectPool::UObjectPool()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UObjectPool::BeginPlay()
{
	Super::BeginPlay();
}

void UObjectPool::DoSomething() 
{

}