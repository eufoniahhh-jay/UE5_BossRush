#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCombatComponent.generated.h"

class UAnimMontage;
class ACharacter;
class UPlayerStatComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOSSRUSH_API UPlayerCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerCombatComponent();

    void LightAttack();

    void HandleAttackStart();
    void HandleAttackEnd();

    // Day5. 회피 액션 관련
    void Dodge();

    UFUNCTION(BlueprintCallable, Category = "Combat|Dodge")
    bool IsInvincible() const { return bIsInvincible; }
    UFUNCTION(BlueprintCallable, Category = "Combat|Dodge")
    bool IsDodging() const { return bIsDodging; }

    void SetInvincible(bool bNewInvincible);
    void FinishDodge();

    // DAy6. 패링 액션 관련
    void Parry();

    UFUNCTION(BlueprintCallable, Category = "Combat|Parry")
    bool IsParryWindowOpen() const { return bIsParryWindowOpen; }

    UFUNCTION(BlueprintCallable, Category = "Combat|Parry")
    bool IsParrying() const { return bIsParrying; }

    void OpenParryWindow();
    void CloseParryWindow();
    void FinishParry();

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

private:
    // Day5. 회피 액션 관련 변수
    UPROPERTY(EditAnywhere, Category = "Dodge")
    float DodgeStaminaCost = 30.0f;

    UPROPERTY(EditAnywhere, Category = "Dodge")
    float DodgeDuration = 0.65f;

    UPROPERTY(EditAnywhere, Category = "Dodge")
    float InvincibleStartDelay = 0.1f;

    UPROPERTY(EditAnywhere, Category = "Dodge")
    float InvincibleDuration = 0.3f;

    UPROPERTY(EditAnywhere, Category = "Dodge")
    float DodgeStrength = 1000.0f;

    bool bIsDodging = false;
    bool bIsInvincible = false;

    FTimerHandle TimerHandle_DodgeEnd;
    FTimerHandle TimerHandle_InvincibleStart;
    FTimerHandle TimerHandle_InvincibleEnd;

    UPROPERTY()
    UPlayerStatComponent* OwnerStatComponent;

private:
    // Day6. 패링 액션 관련 변수
    UPROPERTY(EditAnywhere, Category = "Parry")
    float ParryWindowStartDelay = 0.05f;

    UPROPERTY(EditAnywhere, Category = "Parry")
    float ParryWindowDuration = 0.25f;

    UPROPERTY(EditAnywhere, Category = "Parry")
    float ParryTotalDuration = 0.6f;

    UPROPERTY(EditAnywhere, Category = "Parry")
    float ParryStaminaCost = 15.0f;

    bool bIsParrying = false;
    bool bIsParryWindowOpen = false;

    FTimerHandle TimerHandle_ParryWindowStart;
    FTimerHandle TimerHandle_ParryWindowEnd;
    FTimerHandle TimerHandle_ParryEnd;
};