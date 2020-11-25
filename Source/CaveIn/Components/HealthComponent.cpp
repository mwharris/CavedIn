#include "HealthComponent.h"
#include "CaveIn/GameModes/CaveInGameMode.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	Health = DefaultHealth;
	// Get a reference to our GameMode from GameplayStatics
	GameModeRef = Cast<ACaveInGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	// Bind OnTakeAnyDamage -> TakeDamage()
	GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::TakeDamage);
}

void UHealthComponent::TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser) 
{
	if (Indestructible || Damage == 0 || Health <= 0) 
	{
		return;
	}
	Health -= Damage;
	if (Health <= 0)
	{
		GameModeRef->ActorDied(GetOwner());
	} 
}

float UHealthComponent::GetHealth() const
{
	return Health;
}

void UHealthComponent::SetHealth(float NewHealth) 
{
	Health = NewHealth;
}

float UHealthComponent::GetDefaultHealth() const
{
	return DefaultHealth;
}

void UHealthComponent::SetDefaultHealth(float NewDefaultHealth) 
{
	DefaultHealth = NewDefaultHealth;
}

bool UHealthComponent::IsDead() const
{
	return Health <= 0.f;
}

void UHealthComponent::SetIndestructible(bool Immortal) 
{
	Indestructible = Immortal;
}