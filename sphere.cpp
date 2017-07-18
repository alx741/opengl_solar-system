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

    Color darken()
    {
        Color newColor = {r, g, b};
        float newR = r - 0.2;
        float newG = g - 0.2;
        float newB = b - 0.2;
        if (newR >= 0) { newColor.r = newR; }
        if (newG >= 0) { newColor.g = newG; }
        if (newB >= 0) { newColor.b = newB; }
        return newColor;
    }
};

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
    GLfloat  x, y, z, w;

    static Point3D zero()
    {
        return {0, 0, 0, 0};
    }

    GLfloat &operator [](int i)
    {
        return *(&x + i);
    }

    const GLfloat operator [](int i) const
    {
        return *(&x + i);
    }
};

struct PointSpherical
{
    GLfloat r, a, p;
};

typedef float Angle;

Angle degToRad(Angle a)
{
    return a * (M_PI / 180.0);
}

Angle radToDeg(Angle a)
{
    return a * (180.0 / M_PI);
}

struct Matrix
{
    Point3D matrix[4] = {0, 0, 0, 0};

    // Identity Matrix
    Matrix()
    {
        matrix[0] = {1, 0, 0, 0};
        matrix[1] = {0, 1, 0, 0};
        matrix[2] = {0, 0, 1, 0};
        matrix[3] = {0, 0, 0, 1};
    }

    Matrix(Point3D v1, Point3D v2, Point3D v3, Point3D v4)
    {
        matrix[0] = v1;
        matrix[1] = v2;
        matrix[2] = v3;
        matrix[3] = v3;
    }

    static Matrix Rx(Angle a)
    {
        Matrix m;
        Angle angle = degToRad(a);
        m[0][0] = 1;
        m[3][3] = 1;
        m[1][1] = cos(angle);
        m[2][2] = cos(angle);
        m[2][1] = -sin(angle);
        m[1][2] = sin(angle);
        return m;
    }

    static Matrix Ry(Angle a)
    {
        Matrix m;
        Angle angle = degToRad(a);
        m[1][1] = 1;
        m[3][3] = 1;
        m[0][0] = cos(angle);
        m[2][2] = cos(angle);
        m[0][2] = -sin(angle);
        m[2][0] = sin(angle);
        return m;
    }

    static Matrix Rz(Angle a)
    {
        Matrix m;
        Angle angle = degToRad(a);
        m[3][3] = 1;
        m[2][2] = 1;
        m[0][0] = cos(angle);
        m[1][1] = cos(angle);
        m[1][0] = -sin(angle);
        m[0][1] = sin(angle);
        return m;
    }

    static Matrix scaleMatrix(float b1, float b2, float b3)
    {
        Matrix m;
        m[0][0] = b1;
        m[1][1] = b2;
        m[2][2] = b3;
        m[3][3] = 1;
        return m;
    }

    static Matrix shiftMatrix(float x, float y, float z)
    {
        Matrix m;
        m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1;
        m[0][3] = x;
        m[1][3] = y;
        m[2][3] = z;
        return m;
    }

    Matrix operator * (Matrix m)
    {
        Matrix res;
        for (int a = 0; a < 4; a++)
        {
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    res[a][i] += matrix[a][j] * m[j][i];
                }
            }
        }
        return res;
    }


    Point3D &operator [](int i)
    {
        return matrix[i];
    }

    operator GLfloat *()
    {
        return static_cast<GLfloat *>(&matrix[0].x);
    }
};

/* static const Matrix zeroMatrix = Matrix({0, 0, 0, 0}, {0, 0, 0, 0}  {0, 0, 0, 0}, {0, 0, 0, 0}); */

const int trianglesPerSphere = 8192;
const int verticesPerTriangle = 3;
const int nAstronomicalObjects = 10;
const int nVertices = trianglesPerSphere * verticesPerTriangle * nAstronomicalObjects;
Point3D vertex[nVertices];
Color colors[nVertices];
int INDEX = 0;


// Encontrar el punto medio entre dos puntos a y b
Point3D midPoint(Point3D a, Point3D b)
{
    Point3D sum = {a.x+b.x, a.y+b.y, a.z+b.z};
    return {sum.x/2, sum.y/2, sum.z/2, 1};
}

Point3D sphericalToCartesian(PointSpherical ps)
{
    Point3D p;
    float pol = ps.p * (M_PI/180);
    float azi = ps.a * (M_PI/180);
    p.x = ps.r * sin(pol) * cos(azi);
    p.y = ps.r * sin(pol) * sin(azi);
    p.z = ps.r * cos(pol);
    p.w = 1;
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


    // Draw with default greenish tones
    void draw()
    {
        vertex[INDEX] = a;
        colors[INDEX] = {0.0, 0.6, 0.0};
        INDEX++;
        vertex[INDEX] = b;
        colors[INDEX] = {0.0, 1.0, 0.0};
        INDEX++;
        vertex[INDEX] = c;
        colors[INDEX] = {0.0, 1.0, 0.0};
        INDEX++;
    }

    // Draw with default greenish tones
    void drawWithColor(Color color)
    {
        vertex[INDEX] = a;
        colors[INDEX] = color.darken();
        INDEX++;
        vertex[INDEX] = b;
        colors[INDEX] = color;
        INDEX++;
        vertex[INDEX] = c;
        colors[INDEX] = color;
        INDEX++;
    }

    static void drawTriangles(vector<Triangle> ts)
    {
        for(auto i = ts.begin(); i != ts.end(); ++i)
        {
            i->draw();
        }
    }

    static void drawTrianglesWithColor(vector<Triangle> ts, Color color)
    {
        for(auto i = ts.begin(); i != ts.end(); ++i)
        {
            i->drawWithColor(color);
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
};

struct Sphere
{
    vector<Triangle> triangles;
    const int defaultDepth = 5; // Changing this will brake vertices number count!

    void draw()
    {
        cout << "*DEBUG* Triangles per sphere: " << triangles.size() << endl;
        /* Triangle::drawTriangles(triangles); */
        Triangle::drawTrianglesWithColor(triangles, {0.7, 0.8, 0.5});
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
    glVertexAttribPointer(position, 4, GL_FLOAT, GL_FALSE, 0, 0);

    // Atributos de color
    GLuint vcolor = glGetAttribLocation(program, "vcolor");
    glEnableVertexAttribArray(vcolor);
    glVertexAttribPointer(vcolor, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*) sizeof(vertex));

    // Tranformaciones
    GLint transMatrix = glGetUniformLocation(program, "trans");
    glUniformMatrix4fv(transMatrix, 1, GL_FALSE,
            Matrix() * Matrix::Rx(40) * Matrix::Ry(20) * Matrix::Rz(20)
            * Matrix::scaleMatrix(0.5, 0.5, 0.5)
            * Matrix::shiftMatrix(1.5, 0, 0));

    // Activar algorimo Z
    glEnable(GL_DEPTH_TEST);
    // Configurar color de fondo
    glClearColor(0.5, 0.6, 1.0, 1.0);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, nVertices);
    glFlush();
}

int main(int argc, char **argv)
{

    Sphere s(0.8);
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
