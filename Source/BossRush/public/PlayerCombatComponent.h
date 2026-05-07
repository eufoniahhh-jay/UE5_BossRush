// PlayerCombatComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCombatComponent.generated.h"

class UAnimMontage;
class ACharacter;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOSSRUSH_API UPlayerCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerCombatComponent();

    void LightAttack();

    void HandleAttackStart();
    void HandleAttackEnd();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditAnywhere, Category = "Combat")
    UAnimMontage* LightAttackMontage;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float AttackDamage = 20.0f;

    UPROPERTY(EditAnywhere, Category = "Combat|Trace")
    float AttackTraceDistance = 180.0f;

    UPROPERTY(EditAnywhere, Category = "Combat|Trace")
    float AttackTraceRadius = 60.0f;

    bool bIsAttacking = false;
    bool bAttackHitAlready = false;

    UPROPERTY()
    ACharacter* OwnerCharacter;

    void PerformAttackTrace();
};