// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Type/MatchTypes.h"
#include "CH4GameInstance.generated.h"

USTRUCT(BlueprintType)
struct FPlayerRoleData
{
	GENERATED_BODY()

	UPROPERTY()
	FString PlayerName;

	UPROPERTY()
	EPlayerRole Role;
};

/**
 * 
 */
UCLASS()
class CH4PROJECT_API UCH4GameInstance : public UGameInstance
{
	GENERATED_BODY()
//현재 서버에서의 변경 값이 실제 클라이언트별 게임인스턴스에 리플리케이션이 되지 않는 현상으로 수정 중.
//로직 보충 예정
public:
	
	EWinTeam FinalWinner = EWinTeam::None;
	//승리한 팀을 게임 인스턴스에 저장하지만 기획 상 승패에 따라 역할군별로 위젯 출력하는 구조라면 로비 위젯에서 관리 불가능
	//만약 매치타입을 인스턴스로 한번 더 저장한다면 전반적인 로직 전체를 수정해야하는 문제 발생.
	//구조 상 로비, 레벨 간의 월드 셋팅에 들어가는 모든 파일이 별도로 존재해서 발생하는 문제.
	//따라서 최종 승자의 파트를 게임모드에서 인스턴스로 복제, 이후 게임인스턴스의 LastRoles와 FinalWinner를 가져와 승리한 역할군을 조회.
	//이후 로비 작업에서 UI를 각각 조건에 맞게 업데이트하면 됨.

	
	//대안으로 플레이어 스테이트의 마지막 롤을 복제한 후, 로비로 돌아간 후 역할군에 맞게 출력하는 대안 구조
	//로비 게임모드에서 콜할 때, 플레이어 ID를 바탕으로 역할을 받는다.
	UPROPERTY()
	TMap<FString, EPlayerRole> LastRoles;

	//매치의 상태에 따라 출력되는 위젯을 달리 구성하면 됨.
	//WaitingToStart엔 로비 위젯을, GameOver 일때는 결과창 위젯을 출력하도록 로직을 수정할 것.
	UPROPERTY(BlueprintReadWrite)
	EMatchTypes LastMatchState = EMatchTypes::WaitingToStart;


	UPROPERTY(BlueprintReadWrite)
	EPlayerRole Role;
};