# 📘 프로젝트 이름
> 

---

## 📖 프로젝트 소개
- **프로젝트 개요**  
  이 프로젝트는 

  프로젝트 제작 기간 : 2025.9.8(월) ~ 2025.10.02(목)

  **게임 실행 로직**

  아래는 게임 실행 흐름을 간략히 나타낸 다이어그램입니다.

![Project-playing-01](https://raw.githubusercontent.com/사용자명/레포명/브랜치명/docs/execution_flow.png)

  실행 로직
- **주요 기능 요약**
  - 

- **사용 라이브러리**
  - 언리얼 블루프린트
  - C++


---

## ⚙️ 개발 환경
- **개발 언어 및 도구**
  - IDE: Visual Studio 2022
  - Compiler: MSVC 14.3X / C++17 이상


---

## 📂 프로젝트 구조
```less
CH4Project
 ┣ src
 ┃ ┗ game
 ┃ ┃ ┗  Charater
 ┃ ┃ ┃ ┗ CH4Character.h / .cpp // 두 플레이어 캐릭터에서 공통적으로 사용할 기반 코드.
 ┃ ┃ ┃ ┣ PoliceCH4Character.h / .cpp // 도둑을 체포하기 위한 코드 작성.
 ┃ ┃ ┃ ┗ ThiefCharacter.h / .cpp // 체포 시 애니메이션 재생을 위한 코드 작성.
 ┃ ┃ ┗  PlayerController
 ┃ ┃ ┃ ┣ CH4PlayerController.h / .cpp // 각 플레이어들의 UI 출력 및 게임 인스턴스의 값을 복제하기 위한 코드.
 ┃ ┃ ┃ ┗ CH4ChatPlayerController.h / .cpp // 로비에서의 준비 및, 로비 UI, 결과 UI 출력을 위한 코드.
 ┃ ┃ ┗  GameLevelGameMode
 ┃ ┃ ┃ ┣ CH4GameMode.h / .cpp // 랜덤 역할 분배, 캐릭터 및 아이템 액터 스폰을 네비매쉬 기준으로 실행, 승리 조건 체크 및 게임 레벨 초기화 후 로비 레벨로 귀환.
 ┃ ┃ ┃ ┣ CH4PlayerState.h / .cpp // 플레이어 역할, 남은 체포 횟수, 최대 체포 횟수, 인게임 UI 실시간 반영을 위한 OnRep 기반 코드 구현.
 ┃ ┃ ┃ ┣ CH4GameState.h / .cpp // 매치 상태, 남은 도둑 수, 남은 경비 수, 스폰할 AI 숫자 코드 구현.
 ┃ ┃ ┃ ┗ CH4GameInstance.h / .cpp // 게임 레벨에서 반영된 최종 결과를 로비 게임모드에서 반영할 수 있도록 중간 데이터 건널목이 되는 
 ┃ ┃ ┗  SpawnVolmunes 
 ┃ ┃ ┃ ┣ BaseSpawnVolmunes.h / .cpp // 플레이어, 아이템, AI 스폰을 위한 기본적인 코드 작성
 ┃ ┃ ┃ ┣ PlayerSpawnVolumes.h / .cpp // 상속받은 위치 함수를 override 해서 플레이어의 스폰 위치를 지정.
 ┃ ┃ ┃ ┣ ItemSpawnVolumes.h / .cpp // Item 스폰 위치를 SpawnArea 내의 무작위 위치로 스폰할 수 있도록 코드 구현.
 ┃ ┃ ┃ ┗ AISpawnVolumes.h/ .cpp // AI 스폰 위치를 SpawnArea 내의 무작위 위치로 스폰할 수 있도록 코드 구현.
 ┃ ┃ ┗  LobbyLevelGameMode
 ┃ ┃ ┃ ┣ CH4ChatGameMode.h / .cpp // 플레이어의 Ready 상태를 체크, 플레이어의 로그인을 체크 후 로비 위젯에 플레이어 리스트를 출력, 모든 플레이어가 준비 완료했을 시, 게임 레벨로 이동.
 ┃ ┃ ┃ ┣ CH4ChatGameState.h / .cpp // 위젯의 플레이어 리스트를 최신화 시키기 위한 코드 구현.
 ┃ ┃ ┃ ┗ CH4ChatPlayerState.h / .cpp // 플레이어들의 Ready 상태를 실시간으로 체크하며, CH4ChatGameMode의 CheckAllPlayersReady 함수를 호출해 게임 시작 및 게임 종료 시 결과 위젯 출력 코드.
 ┃ ┃ ┗  UI
 ┃ ┃ ┃ ┗ CH4UserWidget.h /.cpp // 매치 시간, 남은 도둑, 경비 숫자, 남은 체포 횟수, 킬피드, 인벤토리 UI 등 인게임에서 사용되는 UI의 실시간 업데이트를 진행.
 ┃ ┃ ┃ ┗ CH4ChatUserWidget.h / .cpp // 플레이어들의 Ready 버튼을 바인드, Ready와 Exit 코드 구현.
 ┃ ┃ ┗  AI
 ┃ ┃ ┃ ┗ CH4AIChracter.h / .cpp // 클라이언트로 리플리케이션, 애니메이션 몽타주 실행.
 ┃ ┃ ┃ ┗ CH4AIController.h / .cpp // 2~5초마다 랜덤 이동을 실행하는 코드.
 ┃ ┃ ┗
 ┣ 
 ┗ README.md
 ```

## 핵심 로직/클래스 설명

### 플레이어 캐릭터

- **CH4Character**
   > 두 플레이어 캐릭터의 공통 기반 클래스로, 이동 및 입력 처리, 아이템 사용, 애니메이션 몽타주 설정 관리.
- **PoliceCH4Character**
   > 도둑을 체포하기 위한 입력 구현, 판정 로직을 GameMode의 서버RPC 호출을 통해 실행.  
- **ThiefCharacter**
   > 체포 시 특정 애니메이션을 실행.

---

### 게임 모드 / 상태 (GameMode, GameState, PlayerState, GameInstance)
- **CH4GameMode**  
  > 랜덤 역할 분배, 스폰 관리(플레이어, 아이템, AI). 승리 조건 체크 후 로비로 복귀.  
- **CH4PlayerState**  
  > 역할군, 체포 횟수 상태 관리. `OnRep` 기반으로 UI 실시간 업데이트.  
- **CH4GameState**  
  > 매치 상태(게임중/종료), 남은 도둑·경비·AI 수 `OnRep` 기반으로 실시간 업데이트.  
- **CH4GameInstance**  
  > 게임 결과를 로비로 전달하기 위한 중간 데이터 저장소 역할.  

---

### 스폰 시스템 (SpawnVolumes)
- **BaseSpawnVolumes**  
  > 스폰 기능의 공통 베이스.  
- **PlayerSpawnVolumes**  
  > 플레이어 스폰 위치 지정 (Override).  
- **ItemSpawnVolumes**  
  > 랜덤 아이템 스폰.  
- **AISpawnVolumes**  
  > 랜덤 AI 스폰.  

---

### 로비 전용 (Lobby GameMode)
- **CH4ChatGameMode**  
  > 로비에서 Ready 상태 체크 → 모든 플레이어 준비 완료 시 게임 시작.  
- **CH4ChatGameState**  
  > 로비 플레이어 리스트 실시간 동기화.  
- **CH4ChatPlayerState**  
  > 플레이어 Ready 상태 관리 및 CH4CHatGameMode에 전달 및 CH4GameInstance의 값을 가져와 승패 여부에 맞는 결과 위젯 출력.

---

### UI
- **CH4UserWidget**  
  > 인게임 HUD (매치 시간, 체포 횟수, 남은 도둑·경비 수, 킬피드, 인벤토리 등).  
- **CH4ChatUserWidget**  
  > 로비 UI. Ready 버튼, Exit 버튼 처리. 

## 과제 추가 구현 사항

### **기믹 아이템 구현**

- **BaseItem**
  > UObject 기반으로 아이템 사용 함수 및, 아이템 이름, 아이콘 등 필요 데이터를 선언.
- **ClockItem**
  > MatchTime 값을 조절해 경비는 시간 증가, 도둑은 시간 감소 로직을 실행.
- **CokeItem**
  > 플레이어 캐릭터의 이동속도를 5초간 증가시키는 아이템.
- **TrapItem**
  > 플레이어 캐릭터의 이동을 일시적으로 멈추게 하는 아이템.

---

## 트러블 슈팅

### **발생한 문제**

 - **발생한 문제 1**
   > 문제점
   > 원인
   > 해결한 방법

 - **발생한 문제 1**
   > 문제점
   > 원인
   > 해결한 방법

 - **발생한 문제 1**
   > 문제점
   > 원인
   > 해결한 방법

 - **발생한 문제 1**
   > 문제점
   > 원인
   > 해결한 방법

 - **발생한 문제 1**
   > 문제점
   > 원인
   > 해결한 방법