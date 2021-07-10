# YUZA OS

[![YUZA OS Movie](https://img.youtube.com/vi/BM-LlEVNjWU/0.jpg)](https://youtu.be/BM-LlEVNjWU) 

Feature

* 윈도우 10 또는 그 이상 버전에서 비쥬얼 스튜디오로 운영체제 개발

*  WIN32와 실기에서 동시 동작하는 듀얼 시스템
* 강력한 선행적 디버깅 시스템 탑재
* 미니 윈도우 운영체제
* 자신만의 응용앱을 손쉽게 제작하는 방법 제시

Build Tool

- Visual Studio 2019
- 확장 플러그인 [VSNASM] (https://github.com/ShiftMediaProject/VSNASM)

Build Instruction

* runtime 폴더로 이동해서 runtime.sln을 실행한 다음 빌드한다.
  정상 빌드가 되지 않는 경우 어셈블리 파일 빌드를 위해 필요한 확장 플러그인인 VSNASM이 설치됐는지 확인한다.
* corelib.sln을 실행하고 빌드한다.
* thirdparty 폴더로 이동해서 ThirdParty.sln을 실행한 다음 빌드한다.
* support.sln을 실행하고 빌드한다.
* device.sln을 실행하고 빌드한다.
* thirdParty/SDL/SDL2 폴더로 이동해서 SDL2.sln을 실행한 다음 빌드한다.
* thirdParty/SDL/SDL1CL 폴더로 이동해서 sdlcl.sln을 실행한 다음 빌드한다.
* thirdparty2 폴더로 이동해서 ThirdParty2.sln을 실행한 다음 빌드한다.
* kernel.sln 솔루션을 실행한 다음 빌드한다.

커널의 실행

* 챕터별 솔루션 파일을 열어서 커널을 빌드한다. 각 챕터에 따른 빌드옵션을 설정한다.

* BuildOption.h 파일을 열어서 옵션값을 변경하여 WIN32용 또는 실기용으로 빌드한다.
  * SKY_EMULATOR 1 : WIN32용 앱으로 커널 빌드
    * SKY_CONSOLE_MODE 0 : 콘솔 모드
    * SKY_CONSOLE_MODE 1 : 그래픽 모드
  * SKY_EMULATOR 0 : 가상머신용으로 커널 빌드
    * SKY_CONSOLE_MODE 0 : 콘솔 모드
    * SKY_CONSOLE_MODE 1 : 그래픽 모드			

ThirdParty Port

[YUZA]: <https://docs.google.com/spreadsheets/d/1WhGbZbyi8E4f2RWTuU_Y9-RuUu7yq3mHROZEgb20GD4/edit?usp=sharing>



