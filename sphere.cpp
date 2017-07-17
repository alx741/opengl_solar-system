#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <iostream>
#include <cmath>
#include <vector>

using namespace std;

struct Color
{
    float r, g, b;
};

// Paleta de colores
const Color GREEN_LIGHT = {0.0, 1.0, 0.0};
const Color GREEN_DIM = {0.0, 0.6, 0.0};
const Color GREEN_DARK = {0.0, 0.3, 0.0};
const Color BLACK = {0.0, 0.0, 0.0};

// Estructura y función auxiliar para la lectura, carga y compilación de shaders
typedef struct
{
    GLenum       type;
    const char  *filename;
    GLuint       shader;
} ShaderInfo;
GLuint loadShaders(ShaderInfo *);


struct Point2D
{
    GLfloat  x, y;
};

struct Point3D
{
    GLfloat  x, y, z;
};

struct PointSpherical
{
    GLfloat r, a, p;
};

Point3D vertex[100000];
Color colors[100000];
int INDEX = 0;


// Encontrar el punto medio entre dos puntos a y b
Point3D midPoint(Point3D a, Point3D b)
{
    Point3D sum = {a.x+b.x, a.y+b.y, a.z+b.z};
    return {sum.x/2, sum.y/2, sum.z/2};
}

Point3D sphericalToCartesian(PointSpherical ps)
{
    Point3D p;
    float pol = ps.p * (M_PI/180);
    float azi = ps.a * (M_PI/180);
    p.x = ps.r * sin(pol) * cos(azi);
    p.y = ps.r * sin(pol) * sin(azi);
    p.z = ps.r * cos(pol);
    return p;
}

PointSpherical cartesianToSpherical(Point3D p)
{
    PointSpherical ps;
    ps.r = sqrt(pow(p.x, 2) + pow(p.y, 2) + pow(p.z, 2));
    ps.p = (acos(p.z / ps.r)) * (180/M_PI);
    ps.a = (atan2(p.y, p.x)) * (180/M_PI);
    return ps;
}

Point3D pullToRadius(Point3D p, float r)
{
    PointSpherical ps = cartesianToSpherical(p);
    ps.r = r;
    return sphericalToCartesian(ps);
}

struct Triangle
{
    Point3D a,b,c;

    void draw()
    {
        vertex[INDEX] = a;
        colors[INDEX] = GREEN_DIM;
        INDEX++;
        vertex[INDEX] = b;
        colors[INDEX] = GREEN_LIGHT;
        INDEX++;
        vertex[INDEX] = c;
        colors[INDEX] = GREEN_LIGHT;
        INDEX++;
    }

    static void drawTriangles(vector<Triangle> ts)
    {
        for(auto i = ts.begin(); i != ts.end(); ++i)
        {
            i->draw();
        }
    }

    static void subTrianglesRad(Triangle &t, int depth, float r,
            vector<Triangle> *triangles)
    {
        if (depth == 0)
        {
            triangles->push_back(t);
        }
        else
        {
            Triangle t1 = {
                t.a, pullToRadius(midPoint(t.a, t.b), r),
                pullToRadius(midPoint(t.a, t.c), r) };
            Triangle t2 = {
                t.c, pullToRadius(midPoint(t.c, t.b), r),
                pullToRadius(midPoint(t.c, t.a), r) };
            Triangle t3 = {
                t.b, pullToRadius(midPoint(t.b, t.a), r),
                pullToRadius(midPoint(t.b, t.c), r) };
            Triangle t4 = {
                pullToRadius(midPoint(t.c, t.a), r),
                pullToRadius(midPoint(t.b, t.a), r),
                pullToRadius(midPoint(t.b, t.c), r) };

            subTrianglesRad(t1, depth-1, r, triangles);
            subTrianglesRad(t2, depth-1, r, triangles);
            subTrianglesRad(t3, depth-1, r, triangles);
            subTrianglesRad(t4, depth-1, r, triangles);
        }
    }
};

int  Index = 0;
void drawTriangle(Triangle t)
{
    vertex[Index] = t.a;
    colors[Index] = GREEN_DIM;
    Index++;
    vertex[Index] = t.b;
    colors[Index] = GREEN_LIGHT;
    Index++;
    vertex[Index] = t.c;
    colors[Index] = GREEN_LIGHT;
    Index++;
}

struct Octahedron
{
    Triangle ts[8];

    // Octahedron with center on the origin and every vertex on RADIUS distance
    // from the center
    Octahedron(float radius)
    {
        ts[0] =
        {
            sphericalToCartesian({radius, 0, 90}),
            sphericalToCartesian({radius, 90, 90}),
            sphericalToCartesian({radius, 90, 0})
        };
        ts[1] =
        {
            sphericalToCartesian({radius, 0, 90}),
            sphericalToCartesian({radius, 90, 90}),
            sphericalToCartesian({radius, 90, 180})
        };
        ts[2] =
        {
            sphericalToCartesian({radius, 90, 90}),
            sphericalToCartesian({radius, 180, 90}),
            sphericalToCartesian({radius, 90, 0})
        };
        ts[3] =
        {
            sphericalToCartesian({radius, 90, 90}),
            sphericalToCartesian({radius, 180, 90}),
            sphericalToCartesian({radius, 90, 180})
        };
        ts[4] =
        {
            sphericalToCartesian({radius, 90, 270}),
            sphericalToCartesian({radius, 0, 90}),
            sphericalToCartesian({radius, 90, 0})
        };
        ts[5] =
        {
            sphericalToCartesian({radius, 90, 270}),
            sphericalToCartesian({radius, 180, 90}),
            sphericalToCartesian({radius, 90, 0})
        };
        ts[6] =
        {
            sphericalToCartesian({radius, 90, 270}),
            sphericalToCartesian({radius, 0, 90}),
            sphericalToCartesian({radius, 90, 180})
        };
        ts[7] =
        {
            sphericalToCartesian({radius, 90, 270}),
            sphericalToCartesian({radius, 180, 90}),
            sphericalToCartesian({radius, 90, 180})
        };
    }

    void draw()
    {
        for (int i=0; i<8; i++)
        {
            ts[i].draw();
        }
    }

};

struct Sphere
{
    vector<Triangle> triangles;
    int defaultDepth = 5;

    void draw()
    {
        Triangle::drawTriangles(triangles);
    }

    Sphere(float radius)
    {
        // Initial Octahedron
        Octahedron octahedron(radius);

        // Subdiveide each triangle
        for (int i = 0; i < 8; i++)
        {
            Triangle::subTrianglesRad(octahedron.ts[i], defaultDepth, radius,
                    &triangles);
        }
    }
};


void init(void)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    // Buffer de datos contiene los puntos y colores
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) + sizeof(colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex), vertex);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertex), sizeof(colors), colors);

    // Ubicación de los shaders 3d
    ShaderInfo  shaders[] =
    {
        { GL_VERTEX_SHADER, "shaders/3d.vert" },
        { GL_FRAGMENT_SHADER, "shaders/3d.frag" },
        { GL_NONE, NULL }
    };

    // Cargar y compilar los shaders con la funcion auxiliar
    cout << "Cargando Shaders..." << endl;
    GLuint program = loadShaders(shaders);
    glUseProgram(program);

    // Atributos de posición
    GLuint position = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Atributos de color
    GLuint vcolor = glGetAttribLocation(program, "vcolor");
    glEnableVertexAttribArray(vcolor);
    glVertexAttribPointer(vcolor, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) sizeof(vertex));

    // Activar algorimo Z
    glEnable(GL_DEPTH_TEST);
    // Configurar color de fondo
    glClearColor(0.5, 0.6, 1.0, 1.0);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 100000);
    /* glDrawArrays(GL_LINE_LOOP, 0, 10000); */
    glFlush();
}

int main(int argc, char **argv)
{

    Sphere s(0.3);
    s.draw();

    // Inicialización de la ventana
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
    /* glutInitDisplayMode(GLUT_RGBA); */
    glutInitWindowSize(812, 812);
    glutCreateWindow("Gasket 3D");
    glewInit();
    init();

    // Configuración del callback y loop principal
    glutDisplayFunc(display);
    glutMainLoop();

}

/* Carga y compilación de shaders */
static const GLchar *readShader(const char *filename)
{
    // Leer fichero de shader
    cout << "Cargando el shader: " << filename << endl;
    FILE *infile = fopen(filename, "rb");

    if (infile)
    {
        std::cout << "Shader compilado con exito" << std::endl;
    }
    else
    {
        std::cerr << "Error compilando shader: " << filename << std::endl;
        return NULL;
    }

    // Ubicarse al final del shader para calcular el tamaño
    fseek(infile, 0, SEEK_END);
    int len = ftell(infile);
    // Regresar al inicio del shader
    fseek(infile, 0, SEEK_SET);

    // Leer todo el shader
    GLchar *source = new GLchar[len + 1];
    fread(source, 1, len, infile);
    fclose(infile);

    // Null byte terminator de la cadena
    source[len] = 0;
    return const_cast<const GLchar *>(source);
}

GLuint loadShaders(ShaderInfo *shaders)
{
    if (shaders == NULL)
    {
        return 0;
    }

    GLuint program = glCreateProgram();

    ShaderInfo *entry = shaders;
    // Leer y compilar todos los shaders de la estrucutra
    // Se encuentra el final con una entrada de tipo GL_NONE
    while (entry->type != GL_NONE)
    {
        GLuint shader = glCreateShader(entry->type);
        entry->shader = shader;

        // Leer el shader
        // Eliminarlo si la lectura es nula
        const GLchar *source = readShader(entry->filename);
        if (source == NULL)
        {
            for (entry = shaders; entry->type != GL_NONE; ++entry)
            {
                glDeleteShader(entry->shader);
                entry->shader = 0;
            }

            return 0;
        }

        // Cargar el código del shader y compilarlo
        glShaderSource(shader, 1, &source, NULL);
        delete [] source;
        glCompileShader(shader);
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        // Imprimir log si el shader no pudo ser compilado
        if (!compiled)
        {
            GLsizei len;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
            GLchar *log = new GLchar[len + 1];
            glGetShaderInfoLog(shader, len, &len, log);
            std::cerr << "El shader no pudo se compilado: " << log << std::endl;
            delete [] log;

            return 0;
        }

        glAttachShader(program, shader);
        ++entry;
    }


    // Enlazar el shader
    glLinkProgram(program);
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    // Imprimir log si el shader no pudo ser enlazado
    if (!linked)
    {
        GLsizei len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        GLchar *log = new GLchar[len + 1];
        glGetProgramInfoLog(program, len, &len, log);
        std::cerr << "El shader no pudo ser enlazado" << log << std::endl;
        delete [] log;

        // Eliminar los shaders fallidos
        for (entry = shaders; entry->type != GL_NONE; ++entry)
        {
            glDeleteShader(entry->shader);
            entry->shader = 0;
        }

        return 0;
    }

    return program;
}
