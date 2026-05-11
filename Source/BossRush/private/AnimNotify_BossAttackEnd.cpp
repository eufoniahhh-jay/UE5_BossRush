#include "AnimNotify_BossAttackEnd.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

void UAnimNotify_BossAttackEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
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

    UE_LOG(LogTemp, Warning, TEXT("[AnimNotify] BossAttackEnd"));

    static const FName FunctionName(TEXT("HandleBossAttackEnd"));

    if (UFunction* Function = OwnerActor->FindFunction(FunctionName))
    {
        OwnerActor->ProcessEvent(Function, nullptr);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[AnimNotify] Boss owner has no HandleBossAttackEnd function"));
    }
}