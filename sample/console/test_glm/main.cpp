#include <stdio.h>
#include "GUIConsoleFramework.h"
#include <glm/gtx/transform.hpp>

int main_impl(int argc, char** argv)
{
	glm::mat4 myMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(10.0f, 0.0f, 0.0f));
	glm::vec4 myVector(10.0f, 10.0f, 10.0f, 1.0f);
	glm::vec4 transformedVector = myMatrix * myVector; 
	
	printf("%f %f %f %f \n", myMatrix[0][0], myMatrix[0][1], myMatrix[0][2], myMatrix[0][3]);
	printf("%f %f %f %f \n", myMatrix[1][0], myMatrix[1][1], myMatrix[1][2], myMatrix[1][3]);
	printf("%f %f %f %f \n", myMatrix[2][0], myMatrix[2][1], myMatrix[2][2], myMatrix[2][3]);
	printf("%f %f %f %f \n", myMatrix[3][0], myMatrix[3][1], myMatrix[3][2], myMatrix[3][3]);

	printf("%f %f %f\n", transformedVector.x, transformedVector.y, transformedVector.z);

	glm::mat4 myScalingMatrix = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
	transformedVector = myScalingMatrix * transformedVector; // guess the result

	printf("%f %f %f\n", transformedVector.x, transformedVector.y, transformedVector.z);
	return 0;
}


int main(int argc, char** argv)
{
	GUIConsoleFramework framework;
	return framework.Run(argc, argv, main_impl);

    return 0;
}



