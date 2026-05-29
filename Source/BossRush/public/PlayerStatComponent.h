#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStatComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BOSSRUSH_API UPlayerStatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerStatComponent();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetCurrentHP() const { return CurrentHP; }

    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetMaxHP() const { return MaxHP; }

    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetCurrentStamina() const { return CurrentStamina; }

    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetMaxStamina() const { return MaxStamina; }

    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetHPRatio() const;

    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetStaminaRatio() const;

    UFUNCTION(BlueprintCallable, Category = "Stats")
    float GetStaminaRegen() const;

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void ConsumeStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void RecoverStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void RecoverStaminaByDeltaTime(float DeltaTime);

    // (회피에 사용할) 스태미너가 충분한지 확인하는 함수
    UFUNCTION(BlueprintCallable, Category = "Stats")
    bool HasEnoughStamina(float Amount) const;

   

    // Day19. 강화 효과 적용 함수
    UFUNCTION(BlueprintCallable, Category = "Upgrade")
    void AddMaxHP(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Upgrade")
    void AddMaxStamina(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Upgrade")
    void AddStaminaRegen(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Upgrade")
    void RestoreAllStats();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|HP", meta = (AllowPrivateAccess = "true"))
    float MaxHP = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|HP", meta = (AllowPrivateAccess = "true"))
    float CurrentHP = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Stamina", meta = (AllowPrivateAccess = "true"))
    float MaxStamina = 100.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats|Stamina", meta = (AllowPrivateAccess = "true"))
    float CurrentStamina = 100.0f;

    // 스태미너 회복
    //UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Stamina", meta = (AllowPrivateAccess = "true"))
    //float StaminaRegenPerSecond = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats|Stamina", meta = (AllowPrivateAccess = "true"))
    float StaminaRegen = 12.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
    bool bIsDead = false;

public:
    // Day11. 외부에서 사망 여부를 확인할 수 있는 getter 함수
    UFUNCTION(BlueprintCallable, Category = "Stats")
    bool IsDead() const { return bIsDead; }
};