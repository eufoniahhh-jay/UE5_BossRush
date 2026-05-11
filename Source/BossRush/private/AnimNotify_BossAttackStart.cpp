#include "AnimNotify_BossAttackStart.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

void UAnimNotify_BossAttackStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (!MeshComp)
    {
        return;
    }

    AActor* OwnerActor = MeshComp->GetOwner();
    if (!OwnerActor)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[AnimNotify] BossAttackStart"));

    static const FName FunctionName(TEXT("HandleBossAttackStart"));

    if (UFunction* Function = OwnerActor->FindFunction(FunctionName))
    {
        OwnerActor->ProcessEvent(Function, nullptr);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[AnimNotify] Boss owner has no HandleBossAttackStart function"));
    }
}