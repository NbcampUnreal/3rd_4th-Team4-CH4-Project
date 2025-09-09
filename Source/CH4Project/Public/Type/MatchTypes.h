#pragma once

#include "CoreMinimal.h"
#include "MatchTypes.generated.h"

UENUM(BlueprintType)
enum class EMatchTypes : uint8
{
	WaitingToStart UMETA(DisplayName="WaitingToStart"),
	InProgress    UMETA(DisplayName="InProgress"),
	GameOver      UMETA(DisplayName="GameOver")
};

UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
	Unassigned  UMETA(DisplayName="Unassigned"),
	Police  UMETA(DisplayName="Police"),
	Thief   UMETA(DisplayName="Thief"),
	Citizen UMETA(DisplayName="Citizen")
};

//게임 모드 및, 시간을 추가 변경할 시 추가 수정 가능

//ENUM을 사용한 열거형 방식을 채택했기 때문에 디버그 로그 출력 시 인덱스 순서대로 0, 1, 2, 3...같은 방식으로 출력됨.