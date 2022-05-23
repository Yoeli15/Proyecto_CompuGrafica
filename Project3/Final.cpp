/*---------------------------------------------------------*/
/* ----------------  Proyecto Final -----------------------*/
/*-----------------    2022-2   ---------------------------*/

#include <Windows.h>

#include <glad/glad.h>
#include <glfw3.h>	//main
#include <stdlib.h>		
#include <glm/glm.hpp>	//camera y model
#include <glm/gtc/matrix_transform.hpp>	//camera y model
#include <glm/gtc/type_ptr.hpp>
#include <time.h>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>	//Texture

#define SDL_MAIN_HANDLED
#include <SDL/SDL.h>

#include <shader_m.h>
#include <camera.h>
#include <modelAnim.h>
#include <model.h>
#include <Skybox.h>
#include <iostream>

//#pragma comment(lib, "winmm.lib")

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//void my_input(GLFWwindow *window);
void my_input(GLFWwindow* window, int key, int scancode, int action, int mods);
void animate(void);

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
GLFWmonitor *monitors;

void getResolution(void);

// camera
Camera camera(glm::vec3(0.0f, 10.0f, 90.0f));
float MovementSpeed = 0.1f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
 
// timing
const int FPS = 60;
const int LOOP_TIME = 1000 / FPS; // = 16 milisec // 1000 millisec == 1 sec
double	deltaTime = 0.0f,
		lastFrame = 0.0f;

//Lighting
glm::vec3 lightPosition(0.0f, 4.0f, -10.0f);
glm::vec3 lightDirection(0.0f, -1.0f, -1.0f);

//Posiciones
bool	animacion = false;

int		avanza = 0;

float	movBarco_x = 0.0f,
		movBarco_z = 0.0f,
		orienta = 0.0f,
		rot1 = 0.0f,
		rot2 = -90.0f;//si se cambia el barco va a rotar o cambiar de posición, un ejemplo es ponerle 90.0



//Este enum sirve para identificar en el arreglo de floats los parámetros del velociraptor;
enum RaptorParams
{
	RaptorPistaRadio,
	RaptorPistaAngulo,
	RaptorRotCuerpoY,
	RaptorRotCabezaX,
	RaptorRotCabezaY,
	RaptorRotMandibula,
	RaptorRotBrazos,
	RaptorRotPatas,
	RaptorRotColaX,
	RaptorRotColaY,
	RaptorMaxParams
};

float RaptorParam[RaptorMaxParams] = { 0.0f };

//Este arreglo sirve para poder alterar los valores de incrementos.
float RaptorParamInc[RaptorMaxParams] = { 0.0f };

enum PteroParams
{
	PteroPosX,
	PteroPosY,
	PteroPosZ,
	PteroRotX,
	PteroRotY,
	PteroMandibulaAngulo,
	PteroCabezaRotX,
	PteroCabezaRotY,
	PteroAlaDerRot,
	PteroAlaIzqRot,
	PteroMaxParams
};
float PteroParam[RaptorMaxParams] = { 0.0f,	300.0f,	0.0f,	90.0f,	63.0f,	0.0f,	0.0f,	0.0f,	0.0f,	0.0f };
//Este arreglo sirve para poder alterar los valores de incrementos.
float PteroParamInc[RaptorMaxParams] = { 0.0f };
float PteroKeyFrames[9][RaptorMaxParams] = {
	//X		Y		Z		ROTx	ROTY	MAND	CABX	CABY	ADer	AIzq
	{100.0f,300.0f,	0.0f,	90.0f,	0.0f,	0.0f,	0.0f,	0.0f,	0.0f,	0.0f},
	{100.0f,300.0f,100.0f,	90.0f,	40.0f,	5.0f,	-10.0f,	0.0f,	-40.0f,	40.0f},
	{0.0f,	300.0f,100.0f,	90.0f,	80.0f,	10.0f,	-20.0f,	0.0f,	40.0f,	-40.0f},
	{-100.0f,320.0f,100.0f,	90.0f,	120.0f,	20.0f,	-30.0f,	0.0f,	-40.0f,	40.0f},
	{-100.0f,280.0f,0.0f,	90.0f,	160.0f,	30.0f,	-50.0f,	0.0f,	40.0f,	-40.0f},
	{-100.0f,240.0f,-100.0f,90.0f,	200.0f,	40.0f,	-60.0f,	0.0f,	-40.0f,	40.0f},
	{0.0f,	320.0f,-100.0f,	90.0f,	280.0f,	20.0f,	-20.0f,	0.0f,	40.0f,	-40.0f},
	{100.0f,340.0f,-100.0f,	90.0f,	320.0f,	10.0f,	-10.0f,	0.0f,	-40.0f,	40.0f},
	{100.0f,300.0f,0.0f,	90.0f,	360.0f,	0.0f,	0.0f,	0.0f,	0.0f,	0.0f},
};
#define MAX_FRAMES 9
int i_max_steps = 60;
int i_curr_steps = 0;
typedef struct _frame
{
	//Variables para GUARDAR Key Frames
	float posX;		//Variable para PosicionX
	float posY;		//Variable para PosicionY
	float posZ;		//Variable para PosicionZ
	float RaptorKFParams[RaptorMaxParams];
}FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 3;			//introducir datos
bool play = false;
int playIndex = 0;
int playIndexPtero = 0;
int PteroFrameIndex = 9;

void saveFrame(void)
{
	//printf("frameindex %d\n", FrameIndex);
	std::cout << "Frame Index = " << FrameIndex << std::endl;

	
	for (size_t i = 0; i < RaptorMaxParams; i++)
	{
		KeyFrame[FrameIndex].RaptorKFParams[i] = RaptorParam[i];

		std::cout << " RP[" << i<<"] "<< RaptorParam[i];
	}
	std::cout << std::endl;
	FrameIndex++;
}

void resetElements(void)
{
	

	for (size_t i = 2; i < RaptorMaxParams; i++)
	{
		RaptorParam[i] = KeyFrame[0].RaptorKFParams[i];

	}
}
void ResetPtero(void)
{
	for (size_t i = 0; i < PteroMaxParams; i++)
	{
		PteroParam[i] = PteroKeyFrames[0][i];
	}
}

void interpolation(void)
{
	/*incX = (KeyFrame[playIndex + 1].posX - KeyFrame[playIndex].posX) / i_max_steps;
	incY = (KeyFrame[playIndex + 1].posY - KeyFrame[playIndex].posY) / i_max_steps;
	incZ = (KeyFrame[playIndex + 1].posZ - KeyFrame[playIndex].posZ) / i_max_steps;*/
	for (size_t i = 0; i < RaptorMaxParams; i++)
	{
		RaptorParamInc[i] = (KeyFrame[playIndex + 1].RaptorKFParams[i] - KeyFrame[playIndex].RaptorKFParams[i]) / i_max_steps;

	}
	
}
void pteroInter(void)
{
	for (size_t i = 0; i < PteroMaxParams; i++)
	{
		PteroParamInc[i] = (PteroKeyFrames[playIndexPtero+1][i]- PteroKeyFrames[playIndexPtero][i])/i_max_steps;
	}
}

void animate(void)
{
	if (true)
	{
		if (i_curr_steps >= i_max_steps) //end of animation between frames?
		{
			playIndex++;
			if (playIndex > FrameIndex - 2)	//end of total animation?
			{
				std::cout << "Animation ended" << std::endl;
				//printf("termina anim\n");
				playIndex = 0;
				resetElements();
				//play = false;
			}
			i_curr_steps = 0; //Reset counter
		  //Interpolation
			interpolation();
			playIndexPtero++;
			if (playIndexPtero > PteroFrameIndex - 2)
			{
				playIndexPtero = 0;
				ResetPtero();
			}
			pteroInter();

		}
		else
		{
			//Draw animation
			
			RaptorParam[RaptorPistaAngulo]+=0.1;
			for (size_t i = 0; i < RaptorMaxParams; i++)
			{
				RaptorParam[i] += RaptorParamInc[i];
			}
			for (size_t i = 0; i < PteroMaxParams; i++)
			{
				PteroParam[i] += PteroParamInc[i];
			}

			RaptorParam[RaptorPistaAngulo] += 0.1;
			for (size_t i = 0; i < RaptorMaxParams; i++)
			{
				RaptorParam[i] += RaptorParamInc[i];
			}
			i_curr_steps++;
		}
	}



	//Barco
	if(animacion){
		switch (avanza) {
			case 0://Salida del puerto
				if (movBarco_z <= 250.0f) {
					movBarco_z += 2.0f;
					movBarco_x += 1.0f;
				}
				else
					avanza = 1;
				break;
			case 1:
				if (movBarco_z < 360.0f) {
					movBarco_z += 0.2f;
					orienta += 0.2f;
				}
				else
					avanza = 2;
				break;
			case 2://Superior Izquierda
				if (movBarco_x <= 900.0f){
					orienta = 110.0f;
					movBarco_x += 2.0f;
					movBarco_z -= 3.0f;
				}
				else
					avanza = 3;
				break;
			case 3:
				if (movBarco_x < 930.0f) {
					movBarco_x += 0.2f;
					orienta += 0.2f;
				}
				else
					avanza = 4;
				break;

			case 4://Lateral Izq
				if (movBarco_z >= -1800.0f){
					orienta = 150.0f;
					movBarco_z -= 2.0f;
				}
				else
					avanza = 5;
				break;

			case 5:
				if (movBarco_z >= -1860.0f) {
					movBarco_z -= 0.2f;
					orienta -= 0.2f;
				}
				else
					avanza = 6;
				break;

			case 6:
				if (movBarco_z >= -2050.0f) {
					orienta = 90.0f;
					movBarco_z -= 1.0f;
					movBarco_x += 2.0f;
				}
				else
					avanza = 7;
				break;

			case 7:
				if (movBarco_z >= -2110.0f) {
					movBarco_z -= 0.2f;
					orienta += 0.2f;
				}
				else
					avanza = 8;
				break;

			case 8:
				if (movBarco_z >= -2900.0f) {
					orienta = 150.0f;
					movBarco_z -= 2.0f;
				}
				else
					avanza = 9;
				break;

			case 9:
				if (movBarco_z >= -2980.0f) {
					movBarco_z -= 0.2f;
					orienta += 0.2f;
				}
				else
					avanza = 10;
				break;

			case 10://Inferior
				if (movBarco_x >= -800.0f){
					orienta = -130.0f;
					movBarco_x -= 2.0f;
				}
				else
					avanza = 11;
				break;

			case 11:
				if (movBarco_x >= -900.0f) {
					movBarco_x -= 0.2f;
					orienta += 0.2f;
				}
				else
					avanza = 12;
				break;

			case 12://Lateral Derecha
				if (movBarco_z <= -1500.0f) {
					orienta = -30.0f;
					movBarco_z += 2.0f;
				}
				else
					avanza = 13;
				break;

			case 13:
				if (movBarco_z <= -1460.0f) {
					movBarco_z += 0.2f;
					orienta -= 0.2f;
				}
				else
					avanza = 14;
				break;

			case 14:
				if (movBarco_z <= -1380.0f) {
					orienta = -80.0f;
					movBarco_z += 1.0f;
					movBarco_x -= 2.0f;
				}
				else
					avanza = 15;
				break;

			case 15:
				if (movBarco_z <= -1330.0f) {
					movBarco_z += 0.2f;
					orienta += 0.2f;
				}
				else
					avanza = 16;
				break;

			case 16:
				if (movBarco_z <= 0.0f) {
					orienta = -20.0f;
					movBarco_z += 2.0f;
				}
				else
					avanza = 17;
				break;

			case 17:
				if (movBarco_z <= 100.0f) {
					movBarco_z += 0.2f;
					orienta += 0.2f;
				}
				else
					avanza = 18;
				break;

			case 18://Superior Derecha
				if (movBarco_x <= -70.0f) {
					orienta = 80.0f;
					movBarco_x += 2.0f;
				}
				else
					avanza = 19;
				break;
			
			case 19:
				if (movBarco_x <= 10.0f) {
					movBarco_x += 0.2f;
					orienta -= 0.2f;
				}
				else
					avanza = 20;
				break;
			
			case 20://Regresando al puerto
				if (movBarco_z >= 10.0f) {
					movBarco_z -= 2.0f;
				}
				break;
			default:
				break;
		}
	}
}

void getResolution()
{
	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	SCR_WIDTH = mode->width;
	SCR_HEIGHT = (mode->height) - 80;
}


int main()
{
	KeyFrame[0].RaptorKFParams[0] = 0.0f;
	KeyFrame[0].RaptorKFParams[1] = 0.0f;
	KeyFrame[0].RaptorKFParams[2] = -15.0f;
	KeyFrame[0].RaptorKFParams[3] = 0.0f;
	KeyFrame[0].RaptorKFParams[4] = 18.0f;
	KeyFrame[0].RaptorKFParams[5] = -24.0f;
	KeyFrame[0].RaptorKFParams[6] = -38.0f;
	KeyFrame[0].RaptorKFParams[7] = -48.0f;
	KeyFrame[0].RaptorKFParams[8] = 0.0f;
	KeyFrame[0].RaptorKFParams[9] = 30.0f;
	//{ 0.0f, 0.0f, 15.0f, 0.0f, 18.0f, -24.0f, - 38.0f, -48.0f, 0.0f , 30.0f };
	KeyFrame[1].RaptorKFParams[0] = 0.0f;
	KeyFrame[1].RaptorKFParams[1] = 0.0f;
	KeyFrame[1].RaptorKFParams[2] = 12.0f;
	KeyFrame[1].RaptorKFParams[3] = 0.0f;
	KeyFrame[1].RaptorKFParams[4] = -4.0f;
	KeyFrame[1].RaptorKFParams[5] = -6.0f;
	KeyFrame[1].RaptorKFParams[6] = 42.0f;
	KeyFrame[1].RaptorKFParams[7] = 49.0f;
	KeyFrame[1].RaptorKFParams[8] = 0.0f;
	KeyFrame[1].RaptorKFParams[9] = -12.0f;
	//{ 0.0f, 0.0f, 15.0f, 0.0f, 18.0f, -24.0f, - 38.0f, -48.0f, 0.0f , 30.0f };
	KeyFrame[2].RaptorKFParams[0] = 0.0f;
	KeyFrame[2].RaptorKFParams[1] = 0.0f;
	KeyFrame[2].RaptorKFParams[2] = -15.0f;
	KeyFrame[2].RaptorKFParams[3] = 0.0f;
	KeyFrame[2].RaptorKFParams[4] = 18.0f;
	KeyFrame[2].RaptorKFParams[5] = -24.0f;
	KeyFrame[2].RaptorKFParams[6] = -38.0f;
	KeyFrame[2].RaptorKFParams[7] = -48.0f;
	KeyFrame[2].RaptorKFParams[8] = 0.0f;
	KeyFrame[2].RaptorKFParams[9] = 30.0f;
	//{ 0.0f, 0.0f, 15.0f, 0.0f, 18.0f, -24.0f, - 38.0f, -48.0f, 0.0f , 30.0f };
	KeyFrame[3].RaptorKFParams[0] = 0.0f;
	KeyFrame[3].RaptorKFParams[1] = 0.0f;
	KeyFrame[3].RaptorKFParams[2] = 12.0f;
	KeyFrame[3].RaptorKFParams[3] = 0.0f;
	KeyFrame[3].RaptorKFParams[4] = -4.0f;
	KeyFrame[3].RaptorKFParams[5] = -6.0f;
	KeyFrame[3].RaptorKFParams[6] = 42.0f;
	KeyFrame[3].RaptorKFParams[7] = 49.0f;
	KeyFrame[3].RaptorKFParams[8] = 0.0f;
	KeyFrame[3].RaptorKFParams[9] = -12.0f;
	//{ 0.0f, 0.0f, 15.0f, 0.0f, 18.0f, -24.0f, - 38.0f, -48.0f, 0.0f , 30.0f };
	
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	// --------------------
	monitors = glfwGetPrimaryMonitor();
	getResolution();

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CGeIHC", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetWindowPos(window, 0, 30);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, my_input);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader staticShader("Shaders/shader_Lights.vs", "Shaders/shader_Lights.fs");
	Shader skyboxShader("Shaders/skybox.vs", "Shaders/skybox.fs");
	Shader animShader("Shaders/anim.vs", "Shaders/anim.fs");

	vector<std::string> faces
	{
		"resources/skybox/right.jpg",
		"resources/skybox/left.jpg",
		"resources/skybox/top.jpg",
		"resources/skybox/bottom.jpg",
		"resources/skybox/front.jpg",
		"resources/skybox/back.jpg"
	};

	Skybox skybox = Skybox(faces);

	// Shader configuration
	// --------------------
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	// load models
	// -----------
	Model isla("resources/objects/Isla/isla.obj");

	Model agua("resources/objects/piso/Piso.obj");

	
	Model Banco1("resources/objects/Bancos/Banco1/old_table.obj");
	Model Banco4("resources/objects/Bancos/Banco4/Banco4.obj");
	Model Arbol1("resources/objects/Arboles/Arbol1.obj");
	Model Arbol2("resources/objects/Arboles/Arbol2.obj");
	Model Cerca("resources/objects/Cerca/Cerca.obj");
	Model Entrada("resources/objects/Entrada/Entrada.obj");
	Model Kiosko("resources/objects/Kiosko/Prueba.obj");
	Model Mesa("resources/objects/Mesa/Mesa.obj");
	Model Resbaladilla("resources/objects/Resbaladilla/Resbaladilla.obj");

	Model Dinosaurio("resources/objects/Dinos/Triceratop/TriceratopMejora.obj");
	Model Juego("resources/objects/Juego/Teeter03.obj");
	Model Lampara("resources/objects/Lamparas/Lampara.obj");

	Model Basura("resources/objects/BotesBasura/Basura.obj");
	Model BasuraIn("resources/objects/BotesBasura/BasuraIn.obj");
	Model Maquina("resources/objects/Maquinas/Maquina.obj");
	Model Helados("resources/objects/CarroHelados/carrito_helado.obj");
	Model Pasamanos("resources/objects/Pasamanos/Prueba.obj");
	Model SubeBaja("resources/objects/SubeBaja/SubeBaja.obj");


	Model Barco("resources/objects/Barco/Barco.obj");
	Model Barquito("resources/objects/Barco/Barco_scout.obj");
	Model Barquito2("resources/objects/Barco/Barco_speeder.obj");
	Model Puerto("resources/objects/Puerto/Prueba2.obj");
	Model Grada("resources/objects/Gradas/Grada2.obj");
	//Animación Roy
	Model cubo("resources/objects/cubo.obj");
	Model Curva("resources/ObjectsRodrigo/Caminos/Curva.obj");
	Model RaptorCuerpo("resources/ObjectsRodrigo/Raptor/Cuerpo.obj");
	Model RaptorCola("resources/ObjectsRodrigo/Raptor/Cola.obj");
	Model RaptorCabeza("resources/ObjectsRodrigo/Raptor/Cabeza.obj");
	Model RaptorMandibula("resources/ObjectsRodrigo/Raptor/Mandibula.obj");
	Model RaptorBrazoIzq("resources/ObjectsRodrigo/Raptor/BrazoIzq.obj");
	Model RaptorBrazoDer("resources/ObjectsRodrigo/Raptor/BrazoDer.obj");
	Model RaptorPataIzq("resources/ObjectsRodrigo/Raptor/PataIzq.obj");
	Model RaptorPataDer("resources/ObjectsRodrigo/Raptor/PataDer.obj");

	ModelAnim Aplauso1("resources/objects/Aplausos/1/Standing Clap.dae");
	Aplauso1.initShaders(animShader.ID);
	ModelAnim Aplauso2("resources/objects/Aplausos/2/Sitting Clap.dae");
	Aplauso2.initShaders(animShader.ID);
	ModelAnim Aplauso3("resources/objects/Aplausos/3/Standing Clap.dae");
	Aplauso3.initShaders(animShader.ID);
	ModelAnim Aplauso4("resources/objects/Aplausos/4/Sitting Clap.dae");
	Aplauso4.initShaders(animShader.ID);
	ModelAnim Aplauso5("resources/objects/Aplausos/5/Fist Pump.dae");
	Aplauso5.initShaders(animShader.ID);
	ModelAnim Aplauso6("resources/objects/Aplausos/7/Clapping.dae");
	Aplauso6.initShaders(animShader.ID);

	
	Model PteroCabeza("resources/ObjectsRodrigo/Ptero/Cabeza.obj");
	Model PteroMandibula("resources/ObjectsRodrigo/Ptero/Mandibula.obj");
	Model PteroCuerpo("resources/ObjectsRodrigo/Ptero/Cuerpo.obj");
	Model PteroAlaIzq("resources/ObjectsRodrigo/Ptero/AlaIzq.obj");
	Model PteroAlaDer("resources/ObjectsRodrigo/Ptero/AlaDer.obj");
	Model PteroCola("resources/ObjectsRodrigo/Ptero/Cola.obj");
	
	//Model Estatua("resources/objects/DinoParque/Dinosaurio/Stegosaurus.obj");
	
	//Inicialización de KeyFrames
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		KeyFrame[i].posX = 0;
		KeyFrame[i].posY = 0;
		KeyFrame[i].posZ = 0;
	}

	while (!glfwWindowShouldClose(window))
	{
		skyboxShader.setInt("skybox", 0);

		// per-frame time logic
		// --------------------
		lastFrame = SDL_GetTicks();

		// input
		// -----
		//my_input(window);
		animate();

		// render
		// ------
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		staticShader.use();
		//Setup Advanced Lights
		staticShader.setVec3("viewPos", camera.Position);
		staticShader.setVec3("dirLight.direction", lightDirection);
		staticShader.setVec3("dirLight.ambient", glm::vec3(1.0f, 1.0f, 1.0f));
		staticShader.setVec3("dirLight.diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("dirLight.specular", glm::vec3(0.0f, 0.0f, 0.0f));

		staticShader.setVec3("pointLight[0].position", lightPosition);
		staticShader.setVec3("pointLight[0].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[0].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[0].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("pointLight[0].constant", 0.08f);
		staticShader.setFloat("pointLight[0].linear", 0.009f);
		staticShader.setFloat("pointLight[0].quadratic", 0.032f);

		staticShader.setVec3("pointLight[1].position", glm::vec3(-80.0, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[1].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[1].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[1].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("pointLight[1].constant", 1.0f);
		staticShader.setFloat("pointLight[1].linear", 0.009f);
		staticShader.setFloat("pointLight[1].quadratic", 0.032f);

		staticShader.setFloat("material_shininess", 32.0f);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 tmp = glm::mat4(1.0f);
		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		staticShader.setMat4("projection", projection);
		staticShader.setMat4("view", view);

		//// Light
		glm::vec3 lightColor = glm::vec3(0.6f);
		glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.75f);


		// -------------------------------------------------------------------------------------------------------------------------
		// Personaje Animacion
		// -------------------------------------------------------------------------------------------------------------------------
		//Remember to activate the shader with the animation
		animShader.use();
		animShader.setMat4("projection", projection);
		animShader.setMat4("view", view);

		animShader.setVec3("material.specular", glm::vec3(0.5f));
		animShader.setFloat("material.shininess", 32.0f);
		animShader.setVec3("light.ambient", ambientColor);
		animShader.setVec3("light.diffuse", diffuseColor);
		animShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		animShader.setVec3("light.direction", lightDirection);
		animShader.setVec3("viewPos", camera.Position);

		// APLAUSOS1

		model = glm::translate(glm::mat4(1.0f), glm::vec3(50.0f, 15.7f, 251.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(rot1 - 150), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));	// it's a bit too big for our scene, so scale it down
		animShader.setMat4("model", model);
		Aplauso4.Draw(animShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-50.0f, 1.0f, 218.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(rot1 + 180), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.07f));	// it's a bit too big for our scene, so scale it down
		animShader.setMat4("model", model);
		Aplauso6.Draw(animShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-150.0f, 22.7f, 220.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(rot1 + 170), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));	// it's a bit too big for our scene, so scale it down
		animShader.setMat4("model", model);
		Aplauso2.Draw(animShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 8.0f, -218.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.15f));	// it's a bit too big for our scene, so scale it down
		animShader.setMat4("model", model);
		Aplauso1.Draw(animShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(125.0f, 30.0f, -215.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(rot1 - 10), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));	// it's a bit too big for our scene, so scale it down
		animShader.setMat4("model", model);
		Aplauso5.Draw(animShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(175.0f, 0.0f, -115.0f)); // translate it down so it's at the center of the scene
		model = glm::rotate(model, glm::radians(rot1 - 60), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));	// it's a bit too big for our scene, so scale it down
		animShader.setMat4("model", model);
		Aplauso3.Draw(animShader);

		// -------------------------------------------------------------------------------------------------------------------------
		// Escenario
		// -------------------------------------------------------------------------------------------------------------------------
		staticShader.use();//Dibujar las cosas después de este shader
		staticShader.setMat4("projection", projection);
		staticShader.setMat4("view", view);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(20.0f));
		staticShader.setMat4("model", model);
		isla.Draw(staticShader);


		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -52.0f, 0.0f));
		model = glm::scale(model, glm::vec3(2.0f));
		staticShader.setMat4("model", model);
		agua.Draw(staticShader);

		//DINOPARQUE

		model = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f, -0.5f, -1170.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1+70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(475.0f, -0.5f, -1100.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1+70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));

		model = glm::translate(glm::mat4(1.0f), glm::vec3(450.0f, -0.5f, -1030.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1 + 70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(423.0f, 0.0f, -967.5f));//Colocando Entrada
		model = glm::rotate(model, glm::radians(rot1+60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.07f));
		staticShader.setMat4("model", model);
		Entrada.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(350.0f, -0.7f, -880.0f));//Colocando Lámpara
		model = glm::scale(model, glm::vec3(5.0f));
		staticShader.setMat4("model", model);
		Lampara.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(410.0f, -0.7f, -1070.0f));//Colocando Lámpara
		model = glm::scale(model, glm::vec3(5.0f));
		staticShader.setMat4("model", model);
		Lampara.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(490.0f, -1.0f, -930.0f));//Colocando Dinosaurio
		model = glm::rotate(model, glm::radians(rot1-20), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f));
		staticShader.setMat4("model", model);
		Dinosaurio.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(396.0f, -0.5f, -902.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1+70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(371.0f, -0.5f, -832.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1+70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(346.0f, -0.5f, -762.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1+70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(321.0f, -0.5f, -692.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1 + 70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(520.0f, -0.5f, -1000.0f));//Colocando Resbaladilla
		model = glm::rotate(model, glm::radians(rot1 + 50), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));
		staticShader.setMat4("model", model);
		Resbaladilla.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(550.0f, 3.3f, -1060.0f));//Colocando Silla
		model = glm::rotate(model, glm::radians(rot1+90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f));
		staticShader.setMat4("model", model);
		Banco1.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(600.0f, -1.0f, -1060.0f));//Colocando BasuraOrg
		model = glm::scale(model, glm::vec3(4.5f));
		staticShader.setMat4("model", model);
		Basura.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(650.0f, 3.3f, -1060.0f));//Colocando Silla
		model = glm::rotate(model, glm::radians(rot2), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f));
		staticShader.setMat4("model", model);
		Banco1.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(620.0f, -1.0f, -950.0f));//Colocando Mesapicnic
		model = glm::rotate(model, glm::radians(rot1+90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));
		staticShader.setMat4("model", model);
		Mesa.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(700.0f, 0.0f, -1050.0f));//Colocando Arbol
		model = glm::scale(model, glm::vec3(20.0f));
		staticShader.setMat4("model", model);
		Arbol1.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(700.0f, -0.7f, -950.0f));//Colocando Lámpara
		model = glm::scale(model, glm::vec3(5.0f));
		staticShader.setMat4("model", model);
		Lampara.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(720.0f, -1.0f, -1000.0f));//Colocando Silla
		model = glm::rotate(model, glm::radians(rot2), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.12f));
		staticShader.setMat4("model", model);
		Banco4.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(720.0f, -1.0f, -850.0f));//Colocando Maquina
		model = glm::rotate(model, glm::radians(rot1+180), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(13.0f));
		staticShader.setMat4("model", model);
		Maquina.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(700.0f, 0.0f, -760.0f));//Colocando Arbol
		model = glm::scale(model, glm::vec3(20.00f));
		staticShader.setMat4("model", model);
		Arbol2.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(650.0f, -1.0f, -750.0f));//Colocando Mesapicnic
		model = glm::rotate(model, glm::radians(rot1), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));
		staticShader.setMat4("model", model);
		Mesa.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(680.0f, -1.0f, -870.0f));//Colocando Sube y baja
		model = glm::scale(model, glm::vec3(0.15f));
		staticShader.setMat4("model", model);
		SubeBaja.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(750.0f, -0.5f, -1015.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1+90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(750.0f, -0.5f, -941.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1+90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(750.0f, -0.5f, -867.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1 + 90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(750.0f, -0.5f, -643.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1 + 90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(750.0f, -0.5f, -569.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1+90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(750.0f, -0.5f, -495.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1 + 90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(700.0f, -0.7f, -590.0f));//Colocando Lámpara
		model = glm::scale(model, glm::vec3(5.0f));
		staticShader.setMat4("model", model);
		Lampara.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(700.0f, 0.0f, -650.0f));//Colocando Pasamanos
		model = glm::scale(model, glm::vec3(15.0f));
		staticShader.setMat4("model", model);
		Pasamanos.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(700.0f, 3.0f, -540.0f));//Colocando Silla
		model = glm::rotate(model, glm::radians(rot1+90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f));
		staticShader.setMat4("model", model);
		Banco1.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(650.0f, -1.0f, -540.0f));//Colocando Maquina
		model = glm::rotate(model, glm::radians(rot1+90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(13.0f));
		staticShader.setMat4("model", model);
		Maquina.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(600.0f, -1.0f, -540.0f));//Colocando BasuraIn
		model = glm::rotate(model, glm::radians(rot1 + 180), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.5f));
		staticShader.setMat4("model", model);
		BasuraIn.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f, -1.0f, -540.0f));//Colocando Silla
		model = glm::rotate(model, glm::radians(rot1+180), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.12f));
		staticShader.setMat4("model", model);
		Banco4.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f, 7.0f, -680.0f));//Colocando Juego
		model = glm::scale(model, glm::vec3(0.03f));
		staticShader.setMat4("model", model);
		Juego.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(560.0f, -1.0f, -900.0f));//Colocando BasuraOrg
		model = glm::rotate(model, glm::radians(rot1 - 90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.5f));
		staticShader.setMat4("model", model);
		Basura.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(580.0f, -1.0f, -840.0f));//Colocando Kiosko
		model = glm::rotate(model, glm::radians(rot1 + 180), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));
		staticShader.setMat4("model", model);
		Kiosko.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(520.0f, -1.0f, -800.0f));//Colocando BasuraIn
		model = glm::rotate(model, glm::radians(rot1 - 100), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.5f));
		staticShader.setMat4("model", model);
		BasuraIn.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(620.0f, -1.0f, -630.0f));//Colocando Mesapicnic
		model = glm::rotate(model, glm::radians(rot1+90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));
		staticShader.setMat4("model", model);
		Mesa.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(600.0f, -1.0f, -700.0f));//Colocando Sube y baja
		model = glm::scale(model, glm::vec3(0.15f));
		staticShader.setMat4("model", model);
		SubeBaja.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(430.0f, -0.5f, -800.0f));//Colocando Resbaladilla
		model = glm::rotate(model, glm::radians(rot1+70), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));
		staticShader.setMat4("model", model);
		Resbaladilla.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(400.0f, 0.0f, -700.0f));//Colocando Carrito
		model = glm::scale(model, glm::vec3(0.5f));
		staticShader.setMat4("model", model);
		Helados.Draw(staticShader);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-83.0f, -12.0f, 1110.0f));//Colocando Puerto
		model = glm::rotate(model, glm::radians(rot1 - 70), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.4f));
		staticShader.setMat4("model", model);
		Puerto.Draw(staticShader);


		model = glm::translate(glm::mat4(1.0f), glm::vec3(-137.0f, -6.0f, 1280.0f));//Colocando Barquito
		model = glm::rotate(model, glm::radians(rot1 - 155), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(25.0f));
		staticShader.setMat4("model", model);
		Barquito.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(45.0f, -12.0f, 1080.0f));//Colocando Puerto
		model = glm::rotate(model, glm::radians(rot1 - 50), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.4f));
		staticShader.setMat4("model", model);
		Puerto.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(250.0f, -6.0f, 1160.0f));//Colocando Barquito
		model = glm::rotate(model, glm::radians(rot1 + 40), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(25.0f));
		staticShader.setMat4("model", model);
		Barquito2.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(70.0f + movBarco_x, -30.0f, 1530.0f + movBarco_z));//Colocando Barco
		model = glm::rotate(model, glm::radians(rot1 + 20 + orienta), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(80.0f));
		staticShader.setMat4("model", model);
		Barco.Draw(staticShader);

		/*----------------------------------------------------------------------------------
		------------------------------------------------------------------------------------
		---------CREANDO PISTA DE CARRERAS DE VELOCIRAPTORS---------------------------------
		------------------------------------------------------------------------------------
		----------------------------------------------------------------------------------*/
		

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-70.0f, -0.7f, -200.0f));//Colocando Lámpara
		model = glm::scale(model, glm::vec3(5.0f));
		staticShader.setMat4("model", model);
		Lampara.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -230.0f));//Colocando Gradas Frontales
		model = glm::scale(model, glm::vec3(37.0f));
		staticShader.setMat4("model", model);
		Grada.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(110.0f, 0.0f, -200.0f));
		model = glm::rotate(model, glm::radians(rot1 - 30), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(37.0f));
		staticShader.setMat4("model", model);
		Grada.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(200.0f, 0.0f, -120.0f));
		model = glm::rotate(model, glm::radians(rot1 - 60), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(37.0f));
		staticShader.setMat4("model", model);
		Grada.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(250.0f, -0.7f, -80.0f));//Colocando Lámpara
		model = glm::scale(model, glm::vec3(5.0f));
		staticShader.setMat4("model", model);
		Lampara.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(250.0f, 0.0f, -20.0f));//Colocando Maquina
		model = glm::rotate(model, glm::radians(rot1 + 180), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(18.0f));
		staticShader.setMat4("model", model);
		Maquina.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(250.0f, -1.0f, 60.0f));//Colocando BasuraIn
		model = glm::rotate(model, glm::radians(rot1 - 100), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.5f));
		staticShader.setMat4("model", model);
		BasuraIn.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(250.0f, -1.0f, 100.0f));//Colocando BasuraOrg
		model = glm::rotate(model, glm::radians(rot1 - 100), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.5f));
		staticShader.setMat4("model", model);
		Basura.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-270.0f, -0.7f, 40.0f));//Colocando Lámpara
		model = glm::scale(model, glm::vec3(5.0f));
		staticShader.setMat4("model", model);
		Lampara.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-250.0f, 0.0f, 120.0f));//Colocando Gradas Traseras
		model = glm::rotate(model, glm::radians(rot1 + 120), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(37.0f));
		staticShader.setMat4("model", model);
		Grada.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-150.0f, 0.0f, 210.0f));
		model = glm::rotate(model, glm::radians(rot1 + 150), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(37.0f));
		staticShader.setMat4("model", model);
		Grada.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(-40.0f, 0.0f, 250.0f));
		model = glm::rotate(model, glm::radians(rot1 - 190), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(37.0f));
		staticShader.setMat4("model", model);
		Grada.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(70.0f, 0.0f, 250.0f));
		model = glm::rotate(model, glm::radians(rot1 - 170), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(37.0f));
		staticShader.setMat4("model", model);
		Grada.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(150.0f, 0.0f, 240.0f));//Colocando Maquina
		model = glm::rotate(model, glm::radians(rot1 + 110), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(18.0f));
		staticShader.setMat4("model", model);
		Maquina.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(200.0f, 0.0f, 150.0f));//Colocando Carrito
		model = glm::rotate(model, glm::radians(rot1 + 150), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f));
		staticShader.setMat4("model", model);
		Helados.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(190.0f, -0.7f, 220.0f));//Colocando Lámpara
		model = glm::scale(model, glm::vec3(5.0f));
		staticShader.setMat4("model", model);
		Lampara.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.00f, 0.1f, 10.0f));
		model = glm::scale(model, glm::vec3(2.0f));
		staticShader.setMat4("model", model);
		Curva.Draw(staticShader);

		model = glm::rotate(model, glm::radians(RaptorParam[RaptorPistaAngulo]), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(-74.5f+RaptorParam[RaptorPistaRadio], 0.2f, 0.0f));
		model = glm::scale(model, glm::vec3(7.0f));
		model = glm::rotate(model, glm::radians(RaptorParam[RaptorRotCuerpoY]),glm::vec3(0.0f,1.0f,0.0f));
		tmp = model;
		staticShader.setMat4("model", model);
		RaptorCuerpo.Draw(staticShader);

		model = glm::translate(tmp, glm::vec3(0.00f, 1.42f, 0.640f));

		model = glm::rotate(model, glm::radians(RaptorParam[RaptorRotCabezaX]), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(RaptorParam[RaptorRotCabezaY]), glm::vec3(0.0f, 1.0f, 0.0f));
		staticShader.setMat4("model", model);
		RaptorCabeza.Draw(staticShader);

		model = glm::translate(model, glm::vec3(0.00f, 00.0f,0.190f ));
		model = glm::rotate(model, glm::radians(RaptorParam[RaptorRotMandibula]+30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		staticShader.setMat4("model", model);
		RaptorMandibula.Draw(staticShader);


		model = glm::translate(tmp, glm::vec3(0.050f, 1.0f, 0.50f));
		model = glm::rotate(model, glm::radians(-RaptorParam[RaptorRotBrazos]), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		staticShader.setMat4("model", model);
		RaptorBrazoIzq.Draw(staticShader);

		model = glm::translate(tmp, glm::vec3(-0.05f, 1.0f , 0.5f));
		model = glm::rotate(model, glm::radians(RaptorParam[RaptorRotBrazos]), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		staticShader.setMat4("model", model);
		RaptorBrazoDer.Draw(staticShader);

		model = glm::translate(tmp, glm::vec3(0.065f, 1.0f, 0.160f));
		model = glm::rotate(model, glm::radians(-RaptorParam[RaptorRotPatas]), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		staticShader.setMat4("model", model);
		RaptorPataIzq.Draw(staticShader);

		model = glm::translate(tmp, glm::vec3(-0.065f,  1.0f, 0.160f));
		model = glm::rotate(model, glm::radians(RaptorParam[RaptorRotPatas]), glm::vec3(1.0f, 0.0f, 0.0f));
		staticShader.setMat4("model", model);
		RaptorPataDer.Draw(staticShader);

		model = glm::translate(tmp, glm::vec3(0.00f, 1.0f, -0.420f));
		model = glm::rotate(model, glm::radians(RaptorParam[RaptorRotColaX]), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(RaptorParam[RaptorRotColaY]), glm::vec3(0.0f, 1.0f, 0.0f));
		staticShader.setMat4("model", model);
		RaptorCola.Draw(staticShader);
		

		//ZONA RESIDENCIAL


		model = glm::translate(glm::mat4(1.0f), glm::vec3(PteroParam[PteroPosX], PteroParam[PteroPosY], PteroParam[PteroPosZ]));
		model = glm::scale(model, glm::vec3(10.0f));
		model = glm::rotate(model, glm::radians(PteroParam[PteroRotX]), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(PteroParam[PteroRotY]), glm::vec3(0.0f, 0.0f, 1.0f));
		tmp = model;
		staticShader.setMat4("model", model);
		PteroCuerpo.Draw(staticShader);

		model = glm::translate(tmp, glm::vec3(0.00f, 2.3f, 0.0f));

		model = glm::rotate(model, glm::radians(PteroParam[PteroCabezaRotX]), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(PteroParam[PteroCabezaRotY]), glm::vec3(0.0f, 1.0f, 0.0f));
		staticShader.setMat4("model", model);
		PteroCabeza.Draw(staticShader);

		model = glm::translate(model, glm::vec3(0.00f, 0.1f, 0.30f));
		model = glm::rotate(model, glm::radians(PteroParam[PteroMandibulaAngulo]), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f,1.0f,1.5f));
		staticShader.setMat4("model", model);
		PteroMandibula.Draw(staticShader);


		model = glm::translate(tmp, glm::vec3(0.26f, 2.25f, 0.00f));
		model = glm::rotate(model, glm::radians(PteroParam[PteroAlaIzqRot]), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		staticShader.setMat4("model", model);
		PteroAlaIzq.Draw(staticShader);

		model = glm::translate(tmp, glm::vec3(-0.26f, 2.25f, 0.00f));
		model = glm::rotate(model, glm::radians(PteroParam[PteroAlaDerRot]), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		staticShader.setMat4("model", model);
		PteroAlaDer.Draw(staticShader);

		model = glm::translate(tmp, glm::vec3(0.0f,0.76f,-0.15f));
		model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		staticShader.setMat4("model", model);
		PteroCola.Draw(staticShader);


		// -------------------------------------------------------------------------------------------------------------------------
		// Termina Escenario
		// -------------------------------------------------------------------------------------------------------------------------

		//-------------------------------------------------------------------------------------
		// draw skybox as last
		// -------------------
		skyboxShader.use();
		skybox.Draw(skyboxShader, view, projection, camera);

		// Limitar el framerate a 60
		deltaTime = SDL_GetTicks() - lastFrame; // time for full 1 loop
		//std::cout <<"frame time = " << frameTime << " milli sec"<< std::endl;
		if (deltaTime < LOOP_TIME)
		{
			SDL_Delay((int)(LOOP_TIME - deltaTime));
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	skybox.Terminate();

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void my_input(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, (float)deltaTime);
	//Configuración para hacer los keyframes
	/*if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
		RaptorParam[RaptorRotCuerpoY]++;
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		RaptorParam[RaptorRotCuerpoY]--;
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		RaptorParam[RaptorRotCabezaX]++;
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		RaptorParam[RaptorRotCabezaX]--;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		RaptorParam[RaptorRotCabezaY]++;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		RaptorParam[RaptorRotCabezaY]--;
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
		RaptorParam[RaptorRotMandibula]++;
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		RaptorParam[RaptorRotMandibula]--;
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		RaptorParam[RaptorRotBrazos]++;
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		RaptorParam[RaptorRotBrazos]--;
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
		RaptorParam[RaptorRotPatas]++;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		RaptorParam[RaptorRotPatas]--;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		RaptorParam[RaptorRotColaY]--;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		RaptorParam[RaptorRotColaY]++;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		RaptorParam[RaptorRotColaX]++;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		RaptorParam[RaptorRotColaX]--;*/


	//To play KeyFrame animation 
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		if (play == false && (FrameIndex > 1))
		{
			std::cout << "Play animation" << std::endl;
			resetElements();
			//First Interpolation				
			interpolation();

			play = true;
			playIndex = 0;
			i_curr_steps = 0;
		}
		else
		{
			play = false;
			std::cout << "Not enough Key Frames" << std::endl;
		}
	}

	//To Save a KeyFrame
	/*if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		if (FrameIndex < MAX_FRAMES)
		{
			saveFrame();
		}
	}*/


	//Animación Barco
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		animacion ^= true;//operación XOR, lo que tenga la variable le pondrá el valor contrario
	}

	if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		movBarco_x = 0.0f;
		movBarco_z = 0.0f;
		avanza = 0;
		orienta = 0;
		animacion = false;
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}
// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}