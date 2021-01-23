#include "HealthComponent.h"
#include "CaveIn/Actors/CaveTile.h"
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
	UE_LOG(LogTemp, Warning, TEXT("Took Damage..."));
	if (Indestructible)
	{
		UE_LOG(LogTemp, Warning, TEXT("...but I'm indestructible"));
	}
	if (Damage == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("...but damage is 0"));
	}
	if (Health <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("...but health is 0"));
	}
	if (Indestructible || Damage == 0 || Health <= 0) 
	{
		return;
	}
	Health -= Damage;
	UE_LOG(LogTemp, Warning, TEXT("Damage: %f,  Health After: %f"), Damage, Health);
	if (Health <= 0)
	{
		GameModeRef->ActorDied(GetOwner());
	} 
	else if (ACaveTile* HitTile = Cast<ACaveTile>(DamagedActor))
	{
		if (HitTile->GetIsFinalBlock())
		{
			HitTile->UpdateDisplayedHealth(Health);
		}
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