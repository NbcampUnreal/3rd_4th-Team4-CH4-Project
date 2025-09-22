#include "PlayerController/CH4ChatPlayerController.h"
#include "PlayerState/CH4ChatPlayerState.h"
#include "Gamemode/CH4ChatGameMode.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


ACH4ChatPlayerController::ACH4ChatPlayerController()
{
	bReplicates = true;
}

void ACH4ChatPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (IsLocalController() && LobbyWidgetClass)
    {
        if (UUserWidget* LobbyUI = CreateWidget<UUserWidget>(this, LobbyWidgetClass))
        {
            LobbyUI->AddToViewport();
        }
    }
}

// �غ� ���� ȣ��
void ACH4ChatPlayerController::Server_SetReady_Implementation(bool bNewReady)
{
	if (ACH4ChatPlayerState* PS = GetPlayerState<ACH4ChatPlayerState>())
	{
		PS->ServerSetReady(bNewReady);
	}
}

// Ŭ���̾�Ʈ ��� ȭ��
void ACH4ChatPlayerController::ShowResultScreen_Implementation(bool bIsWin)
{
    if (!ResultScreen) return;

    if (UUserWidget* ResultUI = CreateWidget<UUserWidget>(this, ResultScreen))
    {
        ResultUI->AddToViewport();

        if (UImage* ResultImage = Cast<UImage>(ResultUI->GetWidgetFromName(TEXT("ResultScreen"))))
        {
            UTexture2D* WinTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/OutGameUI/Win.Win"));
            UTexture2D* LoseTex = LoadObject<UTexture2D>(nullptr, TEXT("/Game/OutGameUI/Lose.Lose"));
            if (WinTex && LoseTex)
            {
                ResultImage->SetBrushFromTexture(bIsWin ? WinTex : LoseTex);
            }
        }
    }
}

// �κ�� ���ư���
void ACH4ChatPlayerController::ReturnLobby()
{
    UGameplayStatics::OpenLevel(this, FName("LobbyMap"));
}

// ���� ����
void ACH4ChatPlayerController::ExitGame()
{
    if (UWorld* World = GetWorld())
    {
        UKismetSystemLibrary::QuitGame(World, this, EQuitPreference::Quit, false);
    }
}