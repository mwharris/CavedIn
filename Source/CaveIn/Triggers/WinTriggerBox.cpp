#include "WinTriggerBox.h"
#include "CaveIn/Characters/BaseCharacter.h"
#include "CaveIn/GameModes/CaveInGameMode.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AWinTriggerBox::AWinTriggerBox() 
{
    // Bind Actor Overlap event to our OnOverlapBegin function
    OnActorBeginOverlap.AddDynamic(this, &AWinTriggerBox::OnOverlapBegin);
}

void AWinTriggerBox::BeginPlay() 
{
    Super::BeginPlay();
    // Get a reference to the GameMode
    GameModeRef = Cast<ACaveInGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    // Draw our trigger box
    DrawDebugBox(GetWorld(), GetActorLocation(), GetComponentsBoundingBox().GetExtent(), FColor::Yellow, true, -1, 0, 5);
}

void AWinTriggerBox::OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor) 
{
    if (OtherActor && OtherActor != this && Cast<ABaseCharacter>(OtherActor)) 
    {
        GameModeRef->HandleGameOver(true);
    }
}