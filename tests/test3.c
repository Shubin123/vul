#include <stdio.h>
#include <cglm/cglm.h>

int main()
{
    // 4x4 matrix
    mat4 m = {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0};

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            printf("%f ", m[i][j]);
        }
        printf("\n");
    }

    printf("\n");
    mat4 inv;
    glm_mat4_inv(m, inv);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            printf("%f ", inv[i][j]);
        }
        printf("\n");
    }

    mat2 m2 = {
        4.0, 3.0,
        3.0, 2.0};

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            printf("%f ", m2[i][j]);
        }
        printf("\n");
    }

    printf("\n");

    mat2 inv2;

    glm_mat2_inv(m2, inv2);

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            printf("%f ", inv2[i][j]);
        }
        printf("\n");
    }
    

    return 0;
}