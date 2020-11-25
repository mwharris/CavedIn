#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "WinTriggerBox.generated.h"

class ACaveInGameMode;

UCLASS()
class CAVEIN_API AWinTriggerBox : public ATriggerBox
{
	GENERATED_BODY()

public:
	AWinTriggerBox();

	UFUNCTION()
	void OnOverlapBegin(class AActor* OverlappedActor, class AActor* OtherActor);

protected:
	virtual void BeginPlay() override;

private:
	ACaveInGameMode* GameModeRef; 

};
