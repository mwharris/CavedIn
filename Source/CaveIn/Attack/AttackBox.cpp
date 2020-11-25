#include "AttackBox.h"
#include "CaveIn/Actors/CaveTile.h"
#include "CaveIn/Components/HealthComponent.h"
#include "Kismet/GameplayStatics.h"

AAttackBox::AAttackBox() 
{
    Attacking = false;
    // Call Attack() when we overlap an Actor
    OnActorBeginOverlap.AddDynamic(this, &AAttackBox::Attack);
}

void AAttackBox::BeginPlay() 
{
    Super::BeginPlay();    
}

void AAttackBox::Attack(AActor* OverlappedActor, AActor* OtherActor) 
{
    if (OtherActor && OtherActor != this && Attacking) 
    {
        ACaveTile* Tile = Cast<ACaveTile>(OtherActor);
        if (Tile) 
        {
            
        }
    }
}
