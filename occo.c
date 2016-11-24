#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <sndfile.h>
#define TRUE 1
#define FALSE 0



const char * lineshadervert = "\
		#version 130 \n\
//		in vec2 posattrib;\n\
		attribute vec2 posattrib;\n\
		varying float lencalcer;\n\
		void main(){\n\
			gl_Position = vec4(posattrib, 0.0, 1.0);\n\
			lencalcer = gl_VertexID;\n\
		}\n\
";
//Note, i can do the length calculation a little nicer with a geometry shader or something
const char * lineshaderfrag = "\
//		#version 330 \n\
//		out vec4 fragColor;\n\
		varying float lencalcer;\n\
		void main(){\n\
			vec2 offs = vec2(dFdx(lencalcer), dFdy(lencalcer));\n\
			float dist = length(offs);\n\
			gl_FragColor = vec4(0.0, dist, 0.0, 1.0);\n\
		}\n\
";


int shader_printProgramLogStatus(const int id){
        GLint blen = 0;
        glGetProgramiv(id, GL_INFO_LOG_LENGTH, &blen);
        if(blen > 1){
                GLchar * log = (GLchar *) malloc(blen);
                glGetProgramInfoLog(id, blen, 0, log);
                printf("program log: \n%s\n", log);
                free(log);
        }
        return blen;
}
int shader_printShaderLogStatus(const int id){
        GLint blen = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &blen);
        if(blen > 1){
                GLchar * log = (GLchar *) malloc(blen);
                glGetShaderInfoLog(id, blen, 0, log);
                printf("shader log: \n%s\n", log);
                free(log);
        }
        return blen;
}


#define POSATTRIBLOC 0
GLuint progid = 0;
int setupShaders(void){
	GLuint vertid;
	GLuint fragid;
	vertid = glCreateShader(GL_VERTEX_SHADER);
	fragid = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertid, 1, (const GLchar **)&lineshadervert, 0);
	glShaderSource(fragid, 1, (const GLchar **)&lineshaderfrag, 0);
	glCompileShader(vertid);
	glCompileShader(fragid);
	shader_printShaderLogStatus(vertid);
	shader_printShaderLogStatus(fragid);
	progid = glCreateProgram();
	printf("progid is %i\n", progid);
	glAttachShader(progid, vertid);
	glAttachShader(progid, fragid);
	glBindFragDataLocation(progid, 0, "fragColor");
	glBindAttribLocation(progid, POSATTRIBLOC, "posattrib");
	glLinkProgram(progid);
	glDeleteShader(vertid);
	glDeleteShader(fragid);
	shader_printProgramLogStatus(progid);
	glUseProgram(progid);
	return progid;
}


int main(const int argc, const char ** argv){
	if(argc < 2){
		printf("you need a file?\n");
		return 2;
	}

	SNDFILE * fh;
	SF_INFO fi = {0};
	fh = sf_open(argv[1], SFM_READ, &fi);

	if(!fh){
		printf("Unable to read %s with error %s\n", argv[1], sf_strerror(0));
		return 2;
	}
	printf("Channels %i format %x\n", fi.channels, fi.format);


	GLFWwindow * window;
	if(!glfwInit()) return 1;
	glewExperimental = TRUE;
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);

	window = glfwCreateWindow(800, 800, "occo", NULL, NULL);
	if(!window){ glfwTerminate(); return 1; }
	glfwMakeContextCurrent(window);


	GLenum glewError = glewInit();
	if(glewError != GLEW_OK){
		printf("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
		return 1;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);

	setupShaders();
	glUseProgram(progid);
	int numsamp = fi.samplerate / 60;
	printf("numsamp is %i\n", numsamp);

	GLfloat * buffer = malloc(numsamp * fi.channels * sizeof(GLfloat));


//current behavior is fine... uncomment the commented lines to make buggy!
//does not require a vao to be buggy... just anything vbo related
// example of bug happen https://www.youtube.com/watch?v=Z4U6kWDR3cQ
//btw im on caicos (rv910) with mesa 13.0.1
/*
	GLuint vaoid, vboid;
	glGenVertexArrays(1, &vaoid);
	glBindVertexArray(vaoid);
	glGenBuffers(1, &vboid);
	glBindBuffer(vboid, GL_ARRAY_BUFFER);
	printf("buffer id %i\n", vboid);
*/
	glEnableVertexAttribArray(POSATTRIBLOC);
	glVertexAttribPointer(POSATTRIBLOC, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) buffer);
//	glVertexAttribPointer(POSATTRIBLOC, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void *) 0);



	double to, t;
	to = glfwGetTime();
	int frames = 0;
	int readcount = 1;
	while(!glfwWindowShouldClose(window) && readcount){
		glClear(GL_COLOR_BUFFER_BIT);
		readcount = sf_readf_float(fh, buffer, numsamp);
//		glBufferData(GL_ARRAY_BUFFER, readcount * fi.channels * sizeof(GLfloat), buffer, GL_STREAM_DRAW);
		glDrawArrays(GL_LINE_STRIP, 0, numsamp);
		glfwSwapBuffers(window);
		frames++;
		t = glfwGetTime();
		double delta = t - to;
		if(delta > 1.0){ printf("%i frames in %f seconds, or %f fps\n", frames, delta, (double)frames/delta); frames = 0; to = t;}
	}
	sf_close(fh);
	fh = 0;
}
