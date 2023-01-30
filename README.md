# DirectX 12 스터디 프로젝트
![thumbnail](https://user-images.githubusercontent.com/20225459/215422714-5e8fa347-6345-4407-9b4e-109040401e00.png)

## Overview
* 구현 방향
  * game_engine(https://github.com/shlaurant/game-engine) 프로젝트에서 사용하기 위한 directx_renderer archive 생성 프로젝트와 바로 테스트해보기 위한 test 프로젝트로 나뉨.
  * component 레벨의 로직은 directx_renderer 라이브러리에 포함하지 않고 test프로젝트에서 따로 구현.
    * 대표적으로 애니메이션의 경우 interplation 로직이 라이브러리 내장되어있지 않고 test프로젝트에 animator로 따로 들어있다.
  * 참고자료를 통해 이해한바대로 위 2가지 사항을 충족하도록 직접 구현.
    * 다만 FBX_Loader 나 GameTimer같이 DirectX와 직접적으로 관련 없는 코드는 그대로 사용하기도 함.

* 이용 툴
  * IDE: CLion with CMake and Powershell
  * VCS: git

* 참고 자료
  * DirectX12를 이용한 3D 게임 프로그래밍 입문 (https://product.kyobobook.co.kr/detail/S000001057900)
  * [C++과 언리얼로 만드는 MMORPG 게임 개발 시리즈] Part2: 게임 수학과 DirectX12 (https://www.inflearn.com/course/%EC%96%B8%EB%A6%AC%EC%96%BC-3d-mmorpg-2)
  * MSDN 및 Stack Overflow 등 기타 인터넷 자료
  
## Features
### Lighting
* 구성
  * Diffuse Texture + Normal Texture
  * Material
    * Diffuse Albedo
    * Schlick approximation for Fresnel effect
    * Microfacet for roughness
    
![image](https://user-images.githubusercontent.com/20225459/215430270-be9b7045-3ba7-4aa9-a391-7fd2dc152667.png)
* 여러 material 예시. 왼쪽 부터 순서대로
  * 반투명 가산혼합
  * 낮은 체반사, 높은 프레넬 계수
  * 높은 표면 거칠기
  * 체반사, 프레넬 계수, 거칠기 중간

![image](https://user-images.githubusercontent.com/20225459/215432103-26636378-821c-4f00-bd43-ba9242910706.png)
* Diffuse texture 와 normal texture 가 적용된 모습
    
### Shadow
* 개요
  * 엔진에서 그림자를 만들 빛의 vp를 설정해 줄 경우 그림자 mapping을 이용
    * 어떤 종류의 vp를 넣어주느냐에 따라 직사광, 점광, 점적광등 여러 광원에 대한 그림자 생성이 가능
  * 현재 단 하나의 광원 그림자만을 허용

![image](https://user-images.githubusercontent.com/20225459/215433618-bd07eec0-8300-4069-ba31-561ce78b4fb9.png)
* 그림자 mapping에 의해 지형 굴곡에 따라 그림자가 생성된다

### Skybox
![image](https://user-images.githubusercontent.com/20225459/215434329-c6f2c99a-78fd-405d-99ee-45daabdf6e32.png)
* Cubemap을 이용한 skybox
* 카메라 - 물체간의 반사벡터를 통한 환경 반사(해골 머리 참조)

### Mirror
![image](https://user-images.githubusercontent.com/20225459/215434890-afdb3601-45f7-4ed9-b735-1632c189b61a.png)
* 엔진에서 reflection matrix를 넣어주면 모든 물체에 대해 거울 연산을 수행
* Stencil을 이용해 거울 밖으로 반사체가 벗어나지 않도록 함

### Terrain
![lod](https://user-images.githubusercontent.com/20225459/215436992-6667e344-c39a-4ef0-9afc-f1a3c777bac7.gif)
* height texture를 이용해 지형의 높이와 법선을 gpu에서 계산 후 출력
* 테셀레이터를 통한 LOD가 구현되어 있음을 위 이미지에서 확인 가능
  * 확실한 변화를 보기 위해 현재 partitioning 이 integer로 설정됨

### Billboard
![bill](https://user-images.githubusercontent.com/20225459/215463194-2f370ed4-4d1a-46ef-b219-ec62c6417f65.gif)
* 기하셰이더를 통해서 점 하나만 넣어주면 알아서 사각형 생성
* 멀리 있는 물체를 표현할 때 사용할것을 생각하고 시선을 따라 돌아감

### Animation
![anim](https://user-images.githubusercontent.com/20225459/215463421-98330697-947f-4456-a0c4-24e33c1ec436.gif)
* 인터넷에 돌아다니는 fbx를 받아 FBXLoader를 이용해 lib에서 사용하는 형태로 변환
* Debug 모드로 빌드하면 gpu쪽이 느려서 cpu에서 interplation 하게 만들어놨는데 Release로 빌드하니 gpu가 빠른것 같아 넘겨야 되나 고려 중

### Blur
![image](https://user-images.githubusercontent.com/20225459/215466507-4b5bde34-ff82-4b60-ac80-928d2f36a3ce.png)
* 계산 셰이더를 활용한 가우스 블러
* 비슷한 후처리 효과들을 대비해 render(option:uint32)에서 option으로 선택할 수 있도록 준비해 놓음
* Q를 눌러 토글 가능

### Public methods
```
template<typename T> void load_geometries(std::vector<geometry<T>> &geometries)
void load_texture(const std::string &name, const std::wstring &path);
void load_material(const std::vector<std::string> &names, const std::vector<material> &mat);
void init_renderees(const std::vector<std::shared_ptr<renderee>> &);
void update_frame(const frame_globals &, const std::vector<object_constant> &, const std::vector<skin_matrix> &);
```
* load_geometries, load_texture, load_material : 신 초기 리소스 바인딩 용
* init_renderees: renderee struct 를 통해 리소스를 어떻게 묶에서 렌더링 할지 알려준다
* update_frame: 엔진에서 렌더링 패스에 필요한 값을을 매 프레임 업데이트


### Syncronization
* dx12_renderer::wait_cmd_queue_sync: void
  * load_texture 등 신 초기화 과정에서 쓰임
  * gpu가 일을 다 마칠 때 까지 cpu 작업을 하지 않고 기다리는 방식
* frame_resource 클래스
  * fence가 첨가된 circular buffer 방식
  * 버퍼가 다 찰때까지 cpu가 프레임 정보를 계속 밀어 넣을 수 있다
  * 특정 프레임 정보를 덮어 써도 되는지는 fence 정보를 읽어 판단

### MSAA4
* pipeline state object에서 직접 AA값을 박는 방식은 최근 API에 와서 막힌걸로 보인다
* 직접 백버퍼를 렌더 타겟으로 세팅하지 않고 별도의 해상도 4배 텍스처를 렌더 타겟으로 설정 후, 마지막에 서브리소스 리졸브를 해주는 방식으로 해주어야했다.
