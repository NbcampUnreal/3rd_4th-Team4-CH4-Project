# 📘 프로젝트 이름
> **범인은 이 안에 있다**

---

## 📖 프로젝트 소개
- **프로젝트 개요**  
  이 프로젝트는 언리얼 게임 개발의 CH4 언리얼 멀티플레이, 팀프로젝트 과제를 위해 수행한 성과입니다.
  게임의 컨셉은 고가의 미술품이 도난당한 직후를 배경으로 하고 있으며 도둑들을 검거하기 위해 경비원들이 즉시 투입된 상황으로 경찰과 도둑 장르의 게임입니다.
  도둑 측은 제한시간 내 생존, 경비 측은 제한시간 내 모든 도둑을 체포하는 것을 목적으로 합니다.

  프로젝트 제작 기간 : 2025.9.8(월) ~ 2025.10.02(목)

![Project-Project-01](https://github.com/user-attachments/assets/6c1ff4c2-19d1-4e47-a738-d54657cfa80b)


  **게임 실행 로직**

  아래는 게임 실행 흐름을 간략히 나타낸 다이어그램입니다.

![Project-playing-01](https://github.com/user-attachments/assets/7590e026-a44f-4533-9333-bf79c8104f10)

- **주요 기능 요약**
  - 플레이어 인원 수 비례 경찰 숫자가 능동적으로 조정.
  - 기믹 아이템을 사용한 플레이의 다양성을 준비.
  - 게임 현황을 동적으로 HUD에 반영하여 실시간 업데이트.
  - AI 캐릭터들의 무작위 위치로 이동.
  - 멀티플레이 환경 구현 및 승패에 맞는 결과 위젯 출력.


- **레벨 디자인**
  - 미술관 내부 모습

  ![Project-Design-01](https://github.com/user-attachments/assets/97545695-2bff-4570-b766-5bc0259d0013)

  ![Project-Design-02](https://github.com/user-attachments/assets/4dce15fb-ae54-49c4-a032-1807e421529e)

  ![Project-Design-03](https://github.com/user-attachments/assets/86c05f3a-47e8-4aa4-a37e-93abe96ed947)

  ![Project-Design-04](https://github.com/user-attachments/assets/a520c1f1-7f9a-4e1b-b26e-9db573577cd1)


- **캐릭터 애니메이션**

  [캐릭터 이동 애니메이션](https://www.notion.so/28b4b1ca7219800bb2eadfd7c1679b53)
  [경비원의 체포 애니메이션](https://www.notion.so/28b4b1ca72198045a1bec82bf5b94354)
  [도둑의 체포 애니메이션](https://www.notion.so/28b4b1ca7219801c99d5f1bdfc02232b)

  
- **아이템** 
  [기믹 아이템 종류 이미지](https://www.notion.so/28b4b1ca721980ef864ec6f319e2b70a)

  [덫 아이템 이미지](https://www.notion.so/28b4b1ca7219809ba601f6db11d84194)

  [이동속도 증가 아이템 이미지](https://www.notion.so/28b4b1ca7219804a820cc60bc123c90f)

  [시계 아이템 이미지](https://www.notion.so/28b4b1ca721980ec9c11e722afc7178e)


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
 ┃ ┃ ┗  Item
 ┃ ┃ ┃ ┗ BaseItem.h / .cpp // 언리얼 오브젝트를 기반으로 아이템들의 기본 데이터를 구현
 ┃ ┃ ┃ ┗ ClockItem.h / .cpp // 매치 시간의 증감에 영향을 주는 아이템.
 ┃ ┃ ┃ ┗ CokeItem.h / .cpp // 캐릭터의 이동속도 증감에 사용되는 아이템.
 ┃ ┃ ┃ ┗ TrapItem.h / .cpp // 캐릭터의 이동을 멈추게 하는 아이템.

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
   > 문제점 :아이템이 레벨 내 배경 오브젝트 위에 생성되는 현상.
   > 원인 :아이템 스폰 로직이 네비 메쉬 기반으로 스폰되는 구조 중, 배경 오브젝트 위에 네비 메쉬가 배치되어 발생한 현상.
   > 해결한 방법 : 네비 모디파이어 볼륨을 설치해 배경 오브젝트의 위에 네비 메쉬가 생성되지 않도록 하는 것으로 해결.
   ![Project-TravleShooting-01](https://github.com/user-attachments/assets/d46a90ea-7436-4307-886a-e284abde6d1f)

 - **발생한 문제 2**
   > 문제점 : 인벤토리에 아이템을 추가하거나 사용해서 제거될 때, 멀티플레이 환경에서 UI가 갱신되지 않는 문제가 발생.
   > 원인 : 아이템을 실제 획득하는 로직이 서버에서 실행되지만 UI를 업데이트 하는 것이 각 클라이언트별로 담당하기 때문에, 클라이언트 상에서 아이템을 획득한 것을 인식하지 못함.
   > 해결한 방법 : C++ 복제 기반 대신 블루프린트에서 직접 갱신하는 구조로 수정.
     - 1. CreateItem 블루프린트 이벤트 추가 후, 클라이언트로 ClientAddItem RPC를 호출하여, 블루프린트 내에서 인벤토리에 아이템을 추가 후 직접 UI 갱신 노드를 호출하도록 변경함.
     - 2. C++에서는 데이터 관리만 담당하고, UI 반영은 블루프린트 이벤트 기반으로 직접 처리하는 구조로 수정.
     - 3. 클라이언트에서도 아이템 획득 시 즉시 UI가 반영되며 멀티플레이 환경에서 동작하는 것을 확인.

 - **발생한 문제 3**
   > 문제점 : 게임 레벨의 게임모드에서 변경한 값이 로비 레벨로 이동할 경우 값이 유지되지 않는 문제를 해결하기 위해 게임 인스턴스에 값을 저장, 실제 테스트 결과 서버가 멈추는 현상이 발생.
   > 원인 : 언리얼 에디터 상 게임 인스턴스를 별도로 지정해야하지만 해당 작업이 누락되었음.
   > 해결한 방법 : 언리얼 에디터의 게임 인스턴스를 새로 생성한 프로젝트의 게임 인스턴스로 적용 후 정상 동작.

 - **발생한 문제 4**
   > 문제점 : 게임이 끝난 후 승패의 결과 값에 따라 클라이언트별 다른 이미지가 도출되어야 하지만 동일한 패배 이미지가 출력.
   > 원인 : 임의로 클라이언트의 이름을 배정하던 코드가 게임 인스턴스에 저장되지 않고 사용, 초기화 시점 이전에 게임 레벨의 게임모드가 전체 플레이어의 접속을 체크하고 역할 배정 및 게임 시작을 진행했기 때문에 발생.
   > 해결한 방법 : 클라이언트의 이름을 변경하는 코드를 삭제, 최초 값을 그대로 사용하는 방식으로 문제를 해결.

 - **발생한 문제 5**
   > 문제점 : 아이템 스폰 시 땅에 박혀서 스폰되는 현상 발생.
   > 원인 : StartLocation이 아이템 생성될 때 위치를 사용하는데, 해당 위치가 바닥보다 낮거나 Tick에서 Z Offset을 더하면 Sin 값이 음수일 때 바닥으로 내려가기 때문.
   > 해결한 방법 : 라인 트레이스를 통해 땅 위에서 스폰되도록 수정.

 - **발생한 문제 6**
   > 문제점 : 체포를 했을 때 체포 대상과 체포한 장본인의 킬피드를 출력하는 과정을 수행 중 여러 줄의 킬피드 출력에 문제가 발생.
   > 원인 : 버티컬 박스를 만들고 내부에 위젯을 새로 넣어 동적으로 표시되는 킬피드를 구현하였으나, 멀티플레이 상 위젯 업데이트가 복잡해졌음.
   > 해결한 방법 : 위젯을 업데이트 하는 방식이 아닌 텍스트 박스만 생성함으로 `OnRep` 함수를 통해 킬피드가 생성되면 호출해 위젯 위치에 출력하는 방식으로 해결.

 - **발생한 문제 7**
   > 문제점
   > 원인
   > 해결한 방법