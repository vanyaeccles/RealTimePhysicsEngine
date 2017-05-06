#pragma once
#include <glm/vec3.hpp>
#include <vector>


#define NUM_PARTICLES 2000

struct Particle
{
	float mass = 1.0f; //doesn't change

	glm::vec3 position;
	glm::vec3 velocity;

	glm::vec3 colour;
	glm::vec3 force;
};

struct ParticleSystem
{

	int n;

	Particle parti[NUM_PARTICLES];

	float t;
};


class ParticleSystemSim
{

public:


	Particle parti[NUM_PARTICLES];


	float upperRandBound = 10.0f;

	float k_R = 0.8f;
	GLfloat coResFactor = 1.0f + k_R;



	void update(float timestep, bool reverseGrav, bool rightFan, bool leftFan)
	{


		//Per particle, update phase based on forces
		for (int i = 0; i < NUM_PARTICLES; i++) 
		{


			clearForces(parti[i]); // clears accumulated forces

			parti[i].force = computeForces(parti[i], reverseGrav, rightFan, leftFan); // Applies updated force


			//Euler's method
			//Set Updated phase for particle
			parti[i].position += parti[i].velocity * timestep;
			parti[i].velocity += parti[i].force / parti[i].mass * timestep;
			

			//Colour change with velocity
			parti[i].colour[0] -= (parti[i].velocity[1] * 0.001);
			parti[i].colour[1] -= (parti[i].velocity[1] * 0.001);
			parti[i].colour[2] -= (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / upperRandBound));
		}
	}
	


	
	glm::vec3 computeForces(Particle p, bool reverseGrav, bool rightFan, bool leftFan)
	{
		//Get updated forces

		glm::vec3 compForce(0.0f);

		glm::vec3 gravity;

		if (!reverseGrav)
		{
			gravity = glm::vec3(0.0f, -9.81f, 0.0f);
		}
		if (reverseGrav)
		{
			gravity = glm::vec3(0.0f, 9.81f, 0.0f);
		}


		compForce += gravity * p.mass;

		// Drag coefficient for sphere in fluid
		float dragCoefficient = 0.0047f;

		compForce -= dragCoefficient * p.velocity;



		if (rightFan && !leftFan)
		{
			glm::vec3 fan(0.0f, 0.0f, 2.0f);
			compForce += fan * p.mass;
		}
		if (leftFan && !rightFan)
		{
			glm::vec3 fan(0.0f, 0.0f, -2.0f);
			compForce += fan * p.mass;

		}


		return compForce;
	}


	void clearForces(Particle p)
	{
		p.force = glm::vec3(0.0f);
	}



	void particlePlaneCollisionCheck(glm::vec3 planeNormal, glm::vec3 planePoint)
	{

		// Collision Handling - Post Processing Method
		for (int i = 0; i < NUM_PARTICLES; i++)
		{

			//glm::vec3 normal = glm::normalize(glm::cross(planePoint, planePoint2));

			float dotPartiPlane = glm::dot((parti[i].position - planePoint), planeNormal);

			if (dotPartiPlane < 0)
			{
				glm::vec3 newDeltaPos = -(dotPartiPlane * planeNormal);

				//Update Position
				parti[i].position += newDeltaPos;


				glm::vec3 velociPos(parti[i].velocity[0], parti[i].velocity[1], parti[i].velocity[2]);
				float dotPartiPlaneVelocity = glm::dot(velociPos, planeNormal);
				glm::vec3 newDeltaVeloci = -(dotPartiPlaneVelocity * planeNormal);

				//Update Velocity
				parti[i].velocity += coResFactor * newDeltaVeloci;
			}
		}
	}





	void initializeParticles()
	{
		
		for (int i = 0; i < NUM_PARTICLES; i++) 
		{

			parti[i].position[0] = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 20.0f));
			parti[i].position[1] = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / upperRandBound));
			parti[i].position[2] = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / upperRandBound));

			parti[i].velocity[0] = 0;
			parti[i].velocity[1] = 0;
			parti[i].velocity[2] = 0;

			parti[i].force[0] = 0;
			parti[i].force[1] = 0;
			parti[i].force[2] = 0;

			parti[i].colour[0] = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 1));
			parti[i].colour[1] = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 1));
			parti[i].colour[2] = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / 1));

			//partisys.parti[i].mass = (0.001 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (0.1 - 0.0001))));
			parti[i].mass = 1.0f;
		}
	}

};

