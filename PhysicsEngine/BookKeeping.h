#pragma once
#ifndef _BOOKKEEPING_H_
#define _BOOKKEEPING_H_


#include <iostream> // Std. Includes
#include <string>
#include <algorithm>
#include <vector>
#include <GL/glew.h> 
#include <GLFW/glfw3.h> // GLFW
#include <glm/glm.hpp> // GLM Mathematics

#include "SweepAndPrune.h"

float lowerRandBoundPosition = -5.0f;
float upperRandBoundPosition = 5.0f;



class BroadPhaseCollisionDetection
{

public:

	vector<RigidBody> bodies;

	BroadPhaseData sap;

	bool potentiallyColliding[NUM_RIGIDBODIES][NUM_RIGIDBODIES];
	

	//vector<RigidBody> xList;
	//vector<RigidBody> yList;
	//vector<RigidBody> zList;

	vector<SweepAndPruneItem> xList;
	vector<SweepAndPruneItem> yList;
	vector<SweepAndPruneItem> zList;


	//setup the vector of rigid bodies
	void initialise()
	{
		// resize the rigid body vector
		getRBvector();

		//Give the rigid bodies a model and starting position
		setBodies();

		// Initialise to false
		setPotCol();

		//Initialise the lists
		initialiseList();
	}


	void update()
	{
		for (int i = 0; i < NUM_RIGIDBODIES; i++)
		{
			bodies[i].broadPhaseCollision = false;
		}

		//set the list of colliding pairs to all zero 
		sap.setPotColDim();

		//Check positions of the bodies in the lists


		//perform insertion sort on the lists
		sortLists();

		//traverse through each list to set the collisions on each dimension
		traverseLists();

		//checkBoundBoxCollision();
	}


	// Sweep and Prune
#pragma region "Sweep and Prune"

	//fill the lists with rigidbody dimension information, sorts by mins and maxes 
	void initialiseList()
	{
		//Empty the lists
		xList.clear();
		yList.clear();
		zList.clear();

		for (int i = 0; i < NUM_RIGIDBODIES; i++)
		{
			SweepAndPruneItem sapXMax = SweepAndPruneItem(bodies[i].bodyID, bodies[i].maxX);
			SweepAndPruneItem sapXMin = SweepAndPruneItem(bodies[i].bodyID, bodies[i].minX);

			xList.push_back(sapXMax);
			xList.push_back(sapXMin);

			SweepAndPruneItem sapYMax = SweepAndPruneItem(bodies[i].bodyID, bodies[i].maxY);
			SweepAndPruneItem sapYMin = SweepAndPruneItem(bodies[i].bodyID, bodies[i].minY);

			yList.push_back(sapYMax);
			yList.push_back(sapYMin);

			SweepAndPruneItem sapZMax = SweepAndPruneItem(bodies[i].bodyID, bodies[i].maxZ);
			SweepAndPruneItem sapZMin = SweepAndPruneItem(bodies[i].bodyID, bodies[i].minZ);

			zList.push_back(sapZMax);
			zList.push_back(sapZMin);
		}


		//sorts them into ordered list
		insertion_sort(xList);
		insertion_sort(yList);
		insertion_sort(zList);
	}

	
	void checkLists()
	{
		// Go through the list of bodies, reset values if the positions have changed
		for (int i = 0; i < NUM_RIGIDBODIES; i++)
		{
			checkBodyPositionX(bodies[i], xList);
			checkBodyPositionY(bodies[i], yList);
			checkBodyPositionZ(bodies[i], zList);
		}
	}



	//Sorts the sweep and prune list based on mins/maxes
	void sortLists()
	{
		//sorts them into ordered list
		insertion_sort(xList);
		insertion_sort(yList);
		insertion_sort(zList);
	}


	void traverseLists()
	{
		traverseList(xList);
		traverseList(yList);
		traverseList(zList);
	}

	//Traverse list and flag collision on the list dimension
	void traverseList(vector<SweepAndPruneItem> dimList)
	{

		int j;

		for (int i = 0; i < NUM_RIGIDBODIES * 2; i++)
		{

				//j starts at the index above the current index
				if (i != NUM_RIGIDBODIES * 2 - 1)
				{
					j = i + 1;
				}
				// if the list is exhausted, return
				if(i = NUM_RIGIDBODIES * 2 - 1)
				{
					return;
				}
					
				//iterate through the list until the next element is the same body 
				while (dimList[i].ID != dimList[j].ID)
				{
					//These bodies are overlapping on this dimension, set this in the collision information
					sap.overlapStatusX[xList[i].ID][xList[j].ID] = true;
					sap.overlapStatusX[xList[j].ID][xList[i].ID] = true;
					// increment j
					j++;
				}
		}
	}


	

	//insertion sort algorithm (sorts rigidbodies based on object maxX)
	void insertion_sort(vector<SweepAndPruneItem>& arrayb) 
	{
		/*for (int i = 1; i < arrayb.size(); ++i) {
			for (int j = i; j > 0 && arrayb[j - 1].dimValue > arrayb[j].dimValue; --j) {
				std::swap(arrayb[j - 1], arrayb[j]);
			}
		}*/


		//std::sort(arrayb.begin(), arrayb.end(), [](auto i, auto j) {return i.dimValue < j.dimValue; });

		for (int j = 1; j < arrayb.size(); j++)
		{
			GLfloat Key = arrayb[j].dimValue;
			int i = j - 1;
			while (i >= 0 && arrayb[i].dimValue > Key)
			{
				arrayb[i + 1] = arrayb[i];
				i = i - 1;
			}
			arrayb[i + 1].dimValue = Key;
		}
	}


	void checkBodyPositionX(RigidBody rb, vector<SweepAndPruneItem>& arrayb)
	{
		int count = 0;
		
		while (count > 2)
		{
			for (int i = 0; i < arrayb.size(); i++)
			{
				//Find the body in the list, if its the first count it will be the min value
				if (arrayb[i].ID == rb.bodyID && count == 0)
				{
					//Check its dimension
					if (arrayb[i].dimValue == rb.minX)
					{
						// Body position hasn't changed, exit the function
						return;
					}
					else
					{
						//reset the value, it'll get sorted when the list is passed to insertion sort 
						arrayb[i].dimValue = rb.minX;
						count++;
					}
				}
				//Its the second count so it will be the max value
				if (arrayb[i].ID == rb.bodyID && count == 1)
				{
					arrayb[i].dimValue = rb.maxX;
				}
			}
		}
	}

	void checkBodyPositionY(RigidBody rb, vector<SweepAndPruneItem>& arrayb)
	{
		int count = 0;

		while (count > 2)
		{
			for (int i = 0; i < arrayb.size(); i++)
			{
				//Find the body in the list, if its the first iteration it will be the min value
				if (arrayb[i].ID == rb.bodyID && count == 0)
				{
					//Check its dimension
					if (arrayb[i].dimValue == rb.minY)
					{
						// Body position hasn't changed, exit the function
						return;
					}
					else
					{
						//reset the value, it'll get sorted when the list is passed to insertion sort 
						arrayb[i].dimValue = rb.minY;
						count++;
					}
				}
				//Its the second iteration so it will be the max value
				if (arrayb[i].ID == rb.bodyID && count == 1)
				{
					arrayb[i].dimValue = rb.maxY;
				}
			}
		}
	}

	void checkBodyPositionZ(RigidBody rb, vector<SweepAndPruneItem>& arrayb)
	{
		int count = 0;

		while (count > 2)
		{
			for (int i = 0; i < arrayb.size(); i++)
			{
				//Find the body in the list, if its the first iteration it will be the min value
				if (arrayb[i].ID == rb.bodyID && count == 0)
				{
					//Check its dimension
					if (arrayb[i].dimValue == rb.minZ)
					{
						// Body position hasn't changed, exit the function
						return;
					}
					else
					{
						//reset the value, it'll get sorted when the list is passed to insertion sort 
						arrayb[i].dimValue = rb.minZ;
						count++;
					}
				}
				//Its the second iteration so it will be the max value
				if (arrayb[i].ID == rb.bodyID && count == 1)
				{
					arrayb[i].dimValue = rb.maxZ;
				}
			}
		}
	}




	//goes through vector of bodies xyz overlaps and checks for bounding box collision
	void checkBoundBoxCollision()
	{
		for (int i = 0; i < NUM_RIGIDBODIES; i++)
		{
			for (int j = NUM_RIGIDBODIES - 1; j > 0; j--)
			{

				if (i != j)
				{
					bool xCheck = sap.checkOverlapX(i, j);
					bool yCheck = sap.checkOverlapY(i, j);
					bool zCheck = sap.checkOverlapZ(i, j);



					if (xCheck && yCheck && zCheck)
					{
						//Bounding Boxes are overlapping!
						bodies[i].broadPhaseCollision = true;
						bodies[j].broadPhaseCollision = true;
						potentiallyColliding[i][j] = true;

						//std::cout << "Sweep and Prune Bounding Box Test Results: Rigid Body " << i << " and " << j << " are colliding!" << std::endl;
					}

					else if (!(xCheck && yCheck && zCheck))
					{
						potentiallyColliding[i][j] = false;
					}
				}
			}
		}
	}

#pragma endregion








	





#pragma region "BruteForce Bounding Box Collision Test"
	////goes through vector of bodies and checks for bounding sphere collision
	void checkBoundBoxBruteForceCollision()
	{

		for (int i = 0; i < NUM_RIGIDBODIES; i++)
		{
			for (int j = NUM_RIGIDBODIES - 1; j > 0; j--)
			{

				if (i != j)
				{
					if (bruteForceBoxCollision(bodies[i], bodies[j]))
					{
						//Bounding Boxes are overlapping
						bodies[i].broadPhaseCollision = true;
						bodies[j].broadPhaseCollision = true;
						potentiallyColliding[i][j] = true;

						std::cout << "Brute-Force Bounding Box Test Results: Rigid Body " << i << " and " << j << " are colliding!" << std::endl;
					}

					if (!bruteForceBoxCollision(bodies[i], bodies[j]))
					{
						//bodies[i].broadPhaseCollision = false;
						//bodies[j].broadPhaseCollision = false;
						potentiallyColliding[i][j] = false;
					}

					//std::cout << potentiallyColliding[i][j] << std::endl;
				}
			}
		}
	}

	bool bruteForceBoxCollision(RigidBody& rb1, RigidBody& rb2)
	{
		bool xOverlap = rb1.boundBox.posX >= rb2.boundBox.negX && rb1.boundBox.negX <= rb2.boundBox.posX;
		bool yOverlap = rb1.boundBox.posY >= rb2.boundBox.negY && rb1.boundBox.negY <= rb2.boundBox.posY;
		bool zOverlap = rb1.boundBox.posZ >= rb2.boundBox.negZ && rb1.boundBox.negZ <= rb2.boundBox.posZ;

		if (xOverlap && yOverlap && zOverlap)
		{
			return true;
		}

		else
		{
			return false;
		}
	}
#pragma endregion

#pragma region "BoundSphere Collision Test"

	//goes through vector of bodies and checks for bounding sphere collision
	void checkBoundSphereCollision()
	{

		for (int i = 0; i < NUM_RIGIDBODIES; i++)
		{
			for (int j = NUM_RIGIDBODIES - 1; j > 0; j--)
			{

				if (i != j)
				{
					//std::cout << boundSphereCollision(bodies[i], bodies[j]) << std::endl;

					if (boundSphereCollision(bodies[i], bodies[j]))
					{
						//Bounding Spheres are overlapping
						bodies[i].broadPhaseCollision = true;
						bodies[j].broadPhaseCollision = true;
						potentiallyColliding[i][j] = true;

						std::cout << "Bounding Sphere Test Results: Rigid Body " << i << " and " << j << " are colliding!" << std::endl;
					}

					if (!boundSphereCollision(bodies[i], bodies[j]))
					{
						//bodies[i].broadPhaseCollision = false;
						//bodies[j].broadPhaseCollision = false;
						potentiallyColliding[i][j] = false;
					}

					//std::cout << potentiallyColliding[i][j] << std::endl;
				}

			}
		}

	}

	bool boundSphereCollision(RigidBody& rb1, RigidBody& rb2)
	{

		//distance between their centres
		GLfloat d = rb1.dChecker.point2point((rb1.boundSphere.center), (rb2.boundSphere.center));
		//distance of both of their radii
		GLfloat rarb = rb1.boundSphere.radius + rb2.boundSphere.radius;

		if (d < rarb)
		{
			return true;
		}

		else
		{
			return false;
		}

	}

#pragma endregion


#pragma region "Set-up and initialisation"

	void getRBvector()
	{
		//vector<RigidBody> bodies;
		this->bodies.resize(NUM_RIGIDBODIES);

	}


	void setPotCol()
	{
		for (int i = 0; i < NUM_RIGIDBODIES; i++)
			for (int j = 0; j < NUM_RIGIDBODIES; j++)
				potentiallyColliding[i][j] = false;        
	}


	void setBodies()
	{
		for (int i = 0; i < bodies.size(); i++)
		{

			bodies[i].setModel("U:/Physics/Phys4/Stuff/Models/cube.obj");

			
			bodies[i].position[0] = lowerRandBoundPosition + (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / upperRandBoundPosition - lowerRandBoundPosition));
			bodies[i].position[1] = lowerRandBoundPosition + (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / upperRandBoundPosition*0.5 - lowerRandBoundPosition));
			bodies[i].position[2] = lowerRandBoundPosition + (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / upperRandBoundPosition - lowerRandBoundPosition));
			
			
			bodies[i].angVelocity[0] = (lowerRandBoundPosition*0.4) + (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / upperRandBoundPosition*0.6 - (lowerRandBoundPosition*0.4)));
			bodies[i].angVelocity[1] = (lowerRandBoundPosition*0.4) + (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / upperRandBoundPosition*0.6 - (lowerRandBoundPosition*0.4)));
			bodies[i].angVelocity[2] = (lowerRandBoundPosition*0.4) + (static_cast <float> (rand()) / static_cast <float> (RAND_MAX / upperRandBoundPosition*0.6 - (lowerRandBoundPosition*0.4)));


			//give them an integer ID
			bodies[i].bodyID = i;


		}
	}
	
#pragma endregion


};





#endif