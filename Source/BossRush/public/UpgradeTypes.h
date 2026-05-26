#pragma once

#include "CoreMinimal.h"
#include "UpgradeTypes.generated.h"

UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
	IncreaseAttackPower UMETA(DisplayName = "Increase Attack Power"),
	IncreaseMaxHP UMETA(DisplayName = "Increase Max HP"),
	IncreaseMaxStamina UMETA(DisplayName = "Increase Max Stamina"),
	IncreaseStaminaRegen UMETA(DisplayName = "Increase Stamina Regen"),
	ReduceDodgeCost UMETA(DisplayName = "Reduce Dodge Cost"),
	IncreaseParryPostureDamage UMETA(DisplayName = "Increase Parry Posture Damage")
};

USTRUCT(BlueprintType)
struct FUpgradeData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	FName UpgradeId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	EUpgradeType UpgradeType = EUpgradeType::IncreaseAttackPower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Upgrade")
	float Value = 0.0f;
};