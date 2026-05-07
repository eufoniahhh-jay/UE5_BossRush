#include "AnimNotify_AttackStart.h"
#include "GameFramework/Actor.h"
#include "BossRushCharacter.h"
#include "PlayerCombatComponent.h"

void UAnimNotify_AttackStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
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

    ABossRushCharacter* PlayerCharacter = Cast<ABossRushCharacter>(OwnerActor);
    if (!PlayerCharacter)
    {
        return;
    }

    UPlayerCombatComponent* CombatComponent = PlayerCharacter->GetCombatComponent();
    if (!CombatComponent)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[AnimNotify] AttackStart"));

    CombatComponent->HandleAttackStart();
}