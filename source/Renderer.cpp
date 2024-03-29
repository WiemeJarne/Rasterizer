//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"
#include <iostream>
#include <cassert>
using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow)
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;	

	const int amountOfPixels{ m_Width * m_Height };
	m_pDepthBufferPixels = new float[amountOfPixels];
	//fill depth buffer with ifiniy as value
	for (int index{}; index < amountOfPixels; ++index)
	{
		m_pDepthBufferPixels[index] = INFINITY;
	}

	m_pCombustionEffectDiffuseMap = Texture::LoadFromFile("Resources/fireFX_diffuse.png");
	m_pVehicleDiffuseTexture = Texture::LoadFromFile("Resources/vehicle_diffuse.png");
	m_pNormalMap = Texture::LoadFromFile("Resources/vehicle_normal.png");
	m_pSpecularMap = Texture::LoadFromFile("Resources/vehicle_specular.png");
	m_pGlossinessMap = Texture::LoadFromFile("Resources/vehicle_gloss.png");

	m_MeshesWorld = { Mesh{}, Mesh{} };
	m_MeshesWorld[0].primitiveTopology = PrimitiveTopology::TriangeList;
	m_MeshesWorld[1].primitiveTopology = PrimitiveTopology::TriangeList;
	m_MeshesWorld[0].cullMode = CullMode::FrontFaceCulling;
	m_MeshesWorld[1].cullMode = CullMode::NoCulling;

	Utils::ParseOBJ("Resources/vehicle.obj", m_MeshesWorld[0].vertices, m_MeshesWorld[0].indices);
	Utils::ParseOBJ("Resources/fireFX.obj", m_MeshesWorld[1].vertices, m_MeshesWorld[1].indices);

	m_MeshesWorld[0].worldMatrix =
	{
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 50, 1}
	};

	m_MeshesWorld[1].worldMatrix =
	{
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 50, 1}
	};

	//Initialize Camera
	//m_Camera.Initialize(60.f, { .0f,.0f,-10.f }, static_cast<float>(m_Width) / m_Height);
	//m_Camera.Initialize(60.f, { 0.f, 5.f, -30.f }, static_cast<float>(m_Width) / m_Height);
	m_Camera.Initialize(45.f, { 0.f, 0.f, 0.f }, static_cast<float>(m_Width) / m_Height);
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
	delete m_pVehicleDiffuseTexture;
	delete m_pNormalMap;
	delete m_pSpecularMap;
	delete m_pGlossinessMap;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);

	if (m_IsRotating)
	{
		float rotationSpeed{ 0.785398163 }; //in radians
		m_MeshesWorld[0].rotationAngle += rotationSpeed * pTimer->GetElapsed();
		m_MeshesWorld[0].worldMatrix = Matrix::CreateRotationY(m_MeshesWorld[0].rotationAngle) * Matrix::CreateTranslation(m_MeshesWorld[0].worldMatrix.GetTranslation());
		m_MeshesWorld[1].rotationAngle += rotationSpeed * pTimer->GetElapsed();
		m_MeshesWorld[1].worldMatrix = Matrix::CreateRotationY(m_MeshesWorld[0].rotationAngle) * Matrix::CreateTranslation(m_MeshesWorld[0].worldMatrix.GetTranslation());
	}
}

void Renderer::Render()
{
	//@START
	Uint8 redValue{ 100 };
	Uint8 greenValue{ 100 };
	Uint8 blueValue{ 100 };
	std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, INFINITY);
	SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB( m_pBackBuffer->format, redValue, greenValue, blueValue));
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	//RENDER LOGIC
	//W1_Part1();
	//W1_Part2();
	//W1_Part3();
	//W1_Part4();
	//W1_Part5();

	//W2_Part1();
	//W2_Part2();
	//W2_Part3();
	
	//W3_Part1();
	//W3_Part2();

	W4_Part1();

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	for (const Vertex& vertex : vertices_in)
	{
		//transform vertex from world space to view space
		Vector3 viewSpacePos{ m_Camera.viewMatrix.TransformPoint(vertex.position) };
		
		//perspective divide
		Vertex projectedVertex{};
		projectedVertex.position.x = viewSpacePos.x / viewSpacePos.z;
		projectedVertex.position.y = viewSpacePos.y / viewSpacePos.z;
		projectedVertex.position.z = viewSpacePos.z;

		//take camera settings into account
		projectedVertex.position.x /= (m_Width / static_cast<float>(m_Height) * m_Camera.fov);
		projectedVertex.position.y /= m_Camera.fov;

		//transform vertex to screen space
		projectedVertex.position.x = (projectedVertex.position.x + 1) / 2.f * m_Width;
		projectedVertex.position.y = (1 - projectedVertex.position.y) / 2.f * m_Height;

		//set color of the vertex
		projectedVertex.color = vertex.color;
		//set uv of the vertex
		projectedVertex.uv = vertex.uv;

		//add the vertex to vertices_out
		vertices_out.emplace_back(projectedVertex);
	}
}

void Renderer::VertexTransformationFunction(std::vector<Mesh>& meshes_world)
{
	for (Mesh& mesh : meshes_world)
	{
		mesh.vertices_out.clear();
		Matrix worldViewProjectionMatirx{ mesh.worldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix };

		for (const Vertex& vertex : mesh.vertices)
		{
			Vertex_Out vertexOut{};

			vertexOut.position = worldViewProjectionMatirx.TransformPoint({ vertex.position.x, vertex.position.y, vertex.position.z, 0 });

			//perspective divide
			const float wInversed{ 1.f / vertexOut.position.w };
			vertexOut.position.x *= wInversed;
			vertexOut.position.y *= wInversed;
			vertexOut.position.z *= wInversed;
			vertexOut.position.w = wInversed;

			//set color of the vertex
			vertexOut.color = vertex.color;
			//set uv of the vertex
			vertexOut.uv = vertex.uv;
			//set the normal of the vertex
			vertexOut.normal = mesh.worldMatrix.TransformVector(vertex.normal);
			//set the tangent of the vertex
			vertexOut.tangent = mesh.worldMatrix.TransformVector(vertex.tangent);
			//set the viewDirection of the vertex
			vertexOut.viewDirection = m_Camera.origin - mesh.worldMatrix.TransformPoint(vertex.position);

			mesh.vertices_out.emplace_back(vertexOut);
		}
	}
}

void Renderer::W1_Part1() const
{
	std::vector<Vector3> vertices_ndc
	{
		{ 0.f, 0.5f, 1.f },
		{ 0.5f, -0.5f, 1.f },
		{ -0.5f, -0.5f, 1.f }
	};

	std::vector<Vector3> vertices_screenSpace{};
	vertices_screenSpace.push_back(Vector3());
	vertices_screenSpace.push_back(Vector3());
	vertices_screenSpace.push_back(Vector3());
	vertices_screenSpace[0].x = (vertices_ndc[0].x + 1) / 2.f * m_Width;
	vertices_screenSpace[0].y = (1 - vertices_ndc[0].y) / 2.f * m_Height;
	vertices_screenSpace[1].x = (vertices_ndc[1].x + 1) / 2.f * m_Width;
	vertices_screenSpace[1].y = (1 - vertices_ndc[1].y) / 2.f * m_Height;
	vertices_screenSpace[2].x = (vertices_ndc[2].x + 1) / 2.f * m_Width;
	vertices_screenSpace[2].y = (1 - vertices_ndc[2].y) / 2.f * m_Height;

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			Vector2 pixel{ static_cast<float>(px), static_cast<float>(py) };
				  
			Vector2 p0Pixel{ pixel - Vector2(vertices_screenSpace[0].x, vertices_screenSpace[0].y) };
			Vector2 edge1{ Vector2(vertices_screenSpace[1].x, vertices_screenSpace[1].y) - Vector2(vertices_screenSpace[0].x, vertices_screenSpace[0].y) };
				  
			Vector2 p1ToPixel{ pixel - Vector2(vertices_screenSpace[1].x, vertices_screenSpace[1].y) };
			Vector2 edge2{ Vector2(vertices_screenSpace[2].x, vertices_screenSpace[2].y) - Vector2(vertices_screenSpace[1].x, vertices_screenSpace[1].y) };
				  
			Vector2 p2ToPixel{ pixel - Vector2(vertices_screenSpace[2].x, vertices_screenSpace[2].y) };
			Vector2 edge3{ Vector2(vertices_screenSpace[0].x, vertices_screenSpace[0].y) - Vector2(vertices_screenSpace[2].x, vertices_screenSpace[2].y) };
			

			if (Vector2::Cross(edge1, p0Pixel) > 0
				&& Vector2::Cross(edge2, p1ToPixel) > 0
				&& Vector2::Cross(edge3, p2ToPixel) > 0)
			{
				ColorRGBA finalColor{ 1, 1, 1 };

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}
}

void Renderer::W1_Part2() const
{
	std::vector<Vertex> vertices_world
	{
		{{0.f,2.f,0.f}},
		{{1.f, 0.f, 0.f}},
		{{-1.f, 0.f, 0.f}}
	};
	std::vector<Vertex> transformed_vertices_world{};
	VertexTransformationFunction(vertices_world, transformed_vertices_world);

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			Vector2 pixel{ static_cast<float>(px), static_cast<float>(py) };

			Vector2 v0{ transformed_vertices_world[0].position.x, transformed_vertices_world[0].position.y };
			Vector2 v1{ transformed_vertices_world[1].position.x, transformed_vertices_world[1].position.y };
			Vector2 v2{ transformed_vertices_world[2].position.x, transformed_vertices_world[2].position.y };

			if (Vector2::Cross(v1 - v0, pixel - v0) < 0)
			{
				continue;
			}

			if (Vector2::Cross(v2 - v1, pixel - v1) < 0)
			{
				continue;
			}

			if (Vector2::Cross(v0 - v2, pixel - v2) < 0)
			{
				continue;
			}

			float gradient = px / static_cast<float>(m_Width);
			gradient += py / static_cast<float>(m_Width);
			gradient /= 2.0f;

			ColorRGBA finalColor{ 1, 1, 1 };

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

void Renderer::W1_Part3() const
{
	std::vector<Vertex> vertices_world
	{
		{{0.f, 4.f, 2.f}, {1.f, 0.f, 0.f}},
		{{3.f, -2.f, 2.f}, {0.f, 1.f, 0.f}},
		{{-3.f, -2.f, 2.f}, {0.f, 0.f, 1.f}}
	};
	std::vector<Vertex> transformed_vertices_world{};
	VertexTransformationFunction(vertices_world, transformed_vertices_world);

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			Vector2 pixel{ static_cast<float>(px), static_cast<float>(py) };

			Vector2 v0{ transformed_vertices_world[0].position.x, transformed_vertices_world[0].position.y };
			Vector2 v1{ transformed_vertices_world[1].position.x, transformed_vertices_world[1].position.y };
			Vector2 v2{ transformed_vertices_world[2].position.x, transformed_vertices_world[2].position.y };

			float area{ Vector2::Cross(v1 - v0, v2 - v0) / 2.f };
			float w0{ (Vector2::Cross(v2 - v1, pixel - v1) / 2.f) / area };

			if (w0 < 0)
			{
				continue;
			}

			float w1{ (Vector2::Cross(v0 - v2, pixel - v2) / 2.f) / area };

			if (w1 < 0)
			{
				continue;
			}

			float w2{ (Vector2::Cross(v1 - v0, pixel - v0) / 2.f) / area };

			if (w2 < 0)
			{
				continue;
			}

			ColorRGBA finalColor{ transformed_vertices_world[0].color * w0 + transformed_vertices_world[1].color * w1 + transformed_vertices_world[2].color * w2 };

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
}

void Renderer::W1_Part4() const
{
	std::vector<Vertex> vertices_world
	{
		//triangle 0
		{{0.f, 2.f, 0.f}, {1.f, 0.f, 0.f}},
		{{1.5f, -1.f, 0.f}, {1.f, 0.f, 0.f}},
		{{-1.5f, -1.f, 0.f}, {1.f, 0.f, 0.f}},

		//triangle 1
		{{0.f, 4.f, 2.f}, {1.f, 0.f, 0.f}},
		{{3.f, -2.f, 2.f}, {0.f, 1.f, 0.f}},
		{{-3.f, -2.f, 2.f}, {0.f, 0.f, 1.f}}
	};
	std::vector<Vertex> transformed_vertices_world{};
	VertexTransformationFunction(vertices_world, transformed_vertices_world);
	
	const int amountOfTriangles{ static_cast<int>(transformed_vertices_world.size()) / 3 };
	for (int index{}; index < amountOfTriangles; ++index)
	{
		for (int px{}; px < m_Width; ++px)
		{
			for (int py{}; py < m_Height; ++py)
			{
				Vector2 pixel{ static_cast<float>(px), static_cast<float>(py) };

				Vector2 v0{ transformed_vertices_world[index * 3].position.x, transformed_vertices_world[index * 3].position.y };
				Vector2 v1{ transformed_vertices_world[index * 3 + 1].position.x, transformed_vertices_world[index * 3 + 1].position.y };
				Vector2 v2{ transformed_vertices_world[index * 3 + 2].position.x, transformed_vertices_world[index * 3 + 2].position.y };

				float area{ Vector2::Cross(v1 - v0, v2 - v0) / 2.f };
				float w0{ (Vector2::Cross(v2 - v1, pixel - v1) / 2.f) / area };

				if (w0 < 0)
				{
					continue;
				}

				float w1{ (Vector2::Cross(v0 - v2, pixel - v2) / 2.f) / area };

				if (w1 < 0)
				{
					continue;
				}

				float w2{ (Vector2::Cross(v1 - v0, pixel - v0) / 2.f) / area };

				if (w2 < 0)
				{
					continue;
				}

				const int pixelIndex{ py * m_Width + px };
				const float depthValue{  transformed_vertices_world[index * 3].position.z * w0
									   + transformed_vertices_world[index * 3 + 1].position.z * w1
									   + transformed_vertices_world[index * 3 + 2].position.z * w2 };

				if (depthValue <= m_pDepthBufferPixels[pixelIndex])
				{
					m_pDepthBufferPixels[pixelIndex] = depthValue;
				}
				else continue;

				ColorRGBA finalColor
				{ transformed_vertices_world[index * 3].color * w0
				  + transformed_vertices_world[index * 3 + 1].color * w1
				  + transformed_vertices_world[index * 3 + 2].color * w2 };

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}
}

void Renderer::W1_Part5() const
{
	std::vector<Vertex> vertices_world
	{
		//triangle 0
		{{0.f, 2.f, 0.f}, {1.f, 0.f, 0.f}},
		{{1.5f, -1.f, 0.f}, {1.f, 0.f, 0.f}},
		{{-1.5f, -1.f, 0.f}, {1.f, 0.f, 0.f}},

		//triangle 1
		{{0.f, 4.f, 2.f}, {1.f, 0.f, 0.f}},
		{{3.f, -2.f, 2.f}, {0.f, 1.f, 0.f}},
		{{-3.f, -2.f, 2.f}, {0.f, 0.f, 1.f}}
	};

	std::vector<Vertex> transformed_vertices_world{};
	VertexTransformationFunction(vertices_world, transformed_vertices_world);

	const int amountOfTriangles{ static_cast<int>(transformed_vertices_world.size()) / 3 };
	for (int index{}; index < amountOfTriangles; ++index)
	{
		const Vertex& vertex0{ transformed_vertices_world[index * 3] };
		const Vertex& vertex1{ transformed_vertices_world[index * 3 + 1] };
		const Vertex& vertex2{ transformed_vertices_world[index * 3 + 2] };

		Vector2 v0{ vertex0.position.x, vertex0.position.y };
		Vector2 v1{ vertex1.position.x, vertex1.position.y };
		Vector2 v2{ vertex2.position.x, vertex2.position.y };

		Vector2 v1ToV2{ v2 - v1 };
		Vector2 v2ToV0{ v0 - v2 };
		Vector2 v0ToV1{ v1 - v0 };

		Vector2 pixel{};
		int pixelIndex{};
		float depthValue{};
		ColorRGBA finalColor{};
		float w0{};
		float w1{};
		float w2{};

		const float area{ Vector2::Cross(v0ToV1, v2 - v0) / 2.f };

		Vector2 min = { std::min(v0.x, v1.x), std::min(v0.y, v1.y) };
		min.x = std::min(min.x, v2.x);
		min.x = std::max(min.x, 0.f);
		min.y = std::min(min.y, v2.y);
		min.y = std::max(min.y, 0.f);

		Vector2 max = { std::max(v0.x, v1.x), std::max(v0.y, v1.y) };
		max.x = std::max(max.x, v2.x);
		max.x = std::min(max.x, static_cast<float>(m_Width));
		max.y = std::max(max.y, v2.y);
		max.y = std::min(max.y, static_cast<float>(m_Height));

		for (int px{ static_cast<int>(min.x) }; px < max.x; ++px)
		{
			for (int py{ static_cast<int>(min.y) }; py < max.y; ++py)
			{
				pixel = { static_cast<float>(px), static_cast<float>(py) };

				 w0 = Vector2::Cross(v1ToV2, pixel - v1);

				if (w0 < 0)
				{
					continue;
				}

				w1 = Vector2::Cross(v2ToV0, pixel - v2);

				if (w1 < 0)
				{
					continue;
				}

				w2 = Vector2::Cross(v0ToV1, pixel - v0);

				if (w2 < 0)
				{
					continue;
				}

				w0 = (w0 / 2.f) / area;
				w1 = (w1 / 2.f) / area;
				w2 = (w2 / 2.f) / area;

				pixelIndex = { py * m_Width + px };
				depthValue = { vertex0.position.z * w0
									   + vertex1.position.z * w1
									   + vertex2.position.z * w2 };

				if (depthValue <= m_pDepthBufferPixels[pixelIndex])
				{
					m_pDepthBufferPixels[pixelIndex] = depthValue;

					finalColor =
					{ vertex0.color * w0
					  + vertex1.color * w1
					  + vertex2.color * w2 };

					//Update Color in Buffer
					finalColor.MaxToOne();

					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
			}
		}
	}
}

void Renderer::W2_Part1() const
{
	std::vector<Mesh> meshes_world
	{
		Mesh
		{
			{
				Vertex{{-3.f, 3.f, -2.f}},
				Vertex{{0.f, 3.f, -2.f}},
				Vertex{{3.f, 3.f, -2.f}},
				Vertex{{-3.f, 0.f, -2.f}},
				Vertex{{0.f, 0.f, -2.f}},
				Vertex{{3.f, 0.f, -2.f}},
				Vertex{{-3.f, -3.f, -2.f}},
				Vertex{{0.f, -3.f, -2.f}},
				Vertex{{3.f, -3.f, -2.f}}
			},

		//1 of the 2 below indices has to be uncommitted together with the PrimitiveTropology::...

		//indices for triangleList
			{
				3, 0, 1,	1, 4, 3,	4, 1, 2,
				2, 5, 4,	6, 3, 4,	4, 7, 6,
				7, 4, 5,	5, 8, 7
			},

			PrimitiveTopology::TriangeList
			
		//indices for triangleStrip
			/*{
				3, 0, 4, 1, 5, 2,
				2, 6,
				6, 3, 7, 4, 8, 5
			},

			PrimitiveTopology::TriangleStrip*/
		}
	};

	const int amountOfMeshes{ static_cast<int>(meshes_world.size()) };
	for (int meshIndex{}; meshIndex < amountOfMeshes; ++meshIndex)
	{
		Mesh mesh{ meshes_world[meshIndex] };

		int amountOfTriangles{};

		if (mesh.primitiveTopology == PrimitiveTopology::TriangeList)
		{
			amountOfTriangles = static_cast<int>(meshes_world[meshIndex].indices.size()) / 3;
		}
		else if(mesh.primitiveTopology == PrimitiveTopology::TriangleStrip)
		{
			amountOfTriangles = static_cast<int>(meshes_world[meshIndex].indices.size()) - 2;

		}

		std::vector<Vertex> transformed_vertices_world{};
		VertexTransformationFunction(mesh.vertices, transformed_vertices_world);

		for (int triangleIndex{}; triangleIndex < amountOfTriangles; ++triangleIndex)
		{
			//the indices to get the index form the mesh.indices vector
			int vertex0Index{}; 
			int vertex1Index{};
			int vertex2Index{};

			if (mesh.primitiveTopology == PrimitiveTopology::TriangeList)
			{
				vertex0Index = triangleIndex * 3;
				vertex1Index = triangleIndex * 3 + 1;
				vertex2Index = triangleIndex * 3 + 2;
			}
			else if (mesh.primitiveTopology == PrimitiveTopology::TriangleStrip)
			{
				vertex0Index = triangleIndex;
				vertex1Index = triangleIndex + 1;
				vertex2Index = triangleIndex + 2;

				if (mesh.indices[vertex0Index] == mesh.indices[vertex1Index]
					|| mesh.indices[vertex1Index] == mesh.indices[vertex2Index]
					|| mesh.indices[vertex2Index] == mesh.indices[vertex0Index])
				{
					continue;
				}
			}

			const Vertex& vertex0{ transformed_vertices_world[mesh.indices[vertex0Index]] };
			const Vertex& vertex1{ transformed_vertices_world[mesh.indices[vertex1Index]] };
			const Vertex& vertex2{ transformed_vertices_world[mesh.indices[vertex2Index]] };

			Vector2 v0{ vertex0.position.x, vertex0.position.y };
			Vector2 v1{ vertex1.position.x, vertex1.position.y };
			Vector2 v2{ vertex2.position.x, vertex2.position.y };

			const float area{ Vector2::Cross(v1 - v0, v2 - v0) / 2.f };

			Vector2 min = { std::min(v0.x, v1.x), std::min(v0.y, v1.y) };
			min.x = std::min(min.x, v2.x);
			min.x = std::max(min.x, 0.f);
			min.y = std::min(min.y, v2.y);
			min.y = std::max(min.y, 0.f);

			Vector2 max = { std::max(v0.x, v1.x), std::max(v0.y, v1.y) };
			max.x = std::max(max.x, v2.x);
			max.x = std::min(max.x, static_cast<float>(m_Width));
			max.y = std::max(max.y, v2.y);
			max.y = std::min(max.y, static_cast<float>(m_Height));

			for (int px{ static_cast<int>(min.x) }; px < max.x; ++px)
			{
				for (int py{ static_cast<int>(min.y) }; py < max.y; ++py)
				{
					Vector2 pixel{ static_cast<float>(px), static_cast<float>(py) };

					const float w0{ (Vector2::Cross(v2 - v1, pixel - v1) / 2.f) / area };

					if (w0 < 0)
					{
						continue;
					}

					const float w1{ (Vector2::Cross(v0 - v2, pixel - v2) / 2.f) / area };

					if (w1 < 0)
					{
						continue;
					}

					const float w2{ (Vector2::Cross(v1 - v0, pixel - v0) / 2.f) / area };

					if (w2 < 0)
					{
						continue;
					}

					const int pixelIndex{ py * m_Width + px };
					const float depthValue{  vertex0.position.z * w0
										   + vertex1.position.z * w1
										   + vertex2.position.z * w2 };

					if (depthValue <= m_pDepthBufferPixels[pixelIndex])
					{
						m_pDepthBufferPixels[pixelIndex] = depthValue;
					}
					else continue;

					ColorRGBA finalColor
					{   vertex0.color * w0
					  + vertex1.color * w1
					  + vertex2.color * w2 };

					//Update Color in Buffer
					finalColor.MaxToOne();

					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
			}
		}
	}
}

void Renderer::W2_Part2() const
{
	std::vector<Mesh> meshes_world
	{
		Mesh
		{
			{
				Vertex{ { -3.f, 3.f, -2.f }, {}, { 0.f, 0.f } },
				Vertex{ { 0.f, 3.f, -2.f }, {}, { 0.5f, 0.f } },
				Vertex{ { 3.f, 3.f, -2.f }, {}, { 1.f, 0.f } },
				Vertex{ { -3.f, 0.f, -2.f }, {}, { 0.f, 0.5f } },
				Vertex{ { 0.f, 0.f, -2.f }, {}, { 0.5f, 0.5f } },
				Vertex{ { 3.f, 0.f, -2.f }, {}, { 1.f, 0.5f } },
				Vertex{ { -3.f, -3.f, -2.f }, {}, { 0.f, 1.f } },
				Vertex{ { 0.f, -3.f, -2.f }, {}, { 0.5f, 1.f } },
				Vertex{ { 3.f, -3.f, -2.f }, {}, { 1.f, 1.f } }
			},

			/*{
				3, 0, 1,	1, 4, 3,	4, 1, 2,
				2, 5, 4,	6, 3, 4,	4, 7, 6,
				7, 4, 5,	5, 8, 7
			},

			PrimitiveTopology::TriangeList*/

			{
				3, 0, 4, 1, 5, 2,
				2, 6,
				6, 3, 7, 4, 8, 5
			},

			PrimitiveTopology::TriangleStrip
		}
	};

	const int amountOfMeshes{ static_cast<int>(meshes_world.size()) };
	for (int meshIndex{}; meshIndex < amountOfMeshes; ++meshIndex)
	{
		Mesh mesh{ meshes_world[meshIndex] };

		int maxCount{};
		int increment{};

		if (mesh.primitiveTopology == PrimitiveTopology::TriangeList)
		{
			increment = 3;
			maxCount = static_cast<int>(meshes_world[meshIndex].indices.size());
		}
		else
		{
			increment = 1;
			maxCount = static_cast<int>(meshes_world[meshIndex].indices.size()) - 2;
		}

		std::vector<Vertex> transformed_vertices_world{};
		VertexTransformationFunction(mesh.vertices, transformed_vertices_world);

		for (int index{}; index < maxCount; index += increment)
		{
			if (mesh.indices[index] == mesh.indices[index + 1]
				|| mesh.indices[index + 1] == mesh.indices[index + 2]
				|| mesh.indices[index + 2] == mesh.indices[index])
			{
				continue;
			}

			Vertex& vertex0{ transformed_vertices_world[mesh.indices[index]] };
			Vertex& vertex1{ transformed_vertices_world[mesh.indices[index + 1]] };
			Vertex& vertex2{ transformed_vertices_world[mesh.indices[index + 2]] };

			const Vector2 v0{ vertex0.position.x, vertex0.position.y };
			const Vector2 v1{ vertex1.position.x, vertex1.position.y };
			const Vector2 v2{ vertex2.position.x, vertex2.position.y };

			float w0{};
			float w1{};
			float w2{};

			Vector2 v1ToV2{ v2 - v1 };
			Vector2 v2ToV0{ v0 - v2 };
			Vector2 v0ToV1{ v1 - v0 };

			const float area{ Vector2::Cross(v1 - v0, v2 - v0) / 2.f };

			Vector2 min = { std::min(v0.x, v1.x), std::min(v0.y, v1.y) };
			min.x = std::min(min.x, v2.x);
			min.x = std::max(min.x, 0.f);
			min.y = std::min(min.y, v2.y);
			min.y = std::max(min.y, 0.f);

			Vector2 max = { std::max(v0.x, v1.x), std::max(v0.y, v1.y) };
			max.x = std::max(max.x, v2.x);
			max.x = std::min(max.x, static_cast<float>(m_Width));
			max.y = std::max(max.y, v2.y);
			max.y = std::min(max.y, static_cast<float>(m_Height));

			for (int px{ static_cast<int>(min.x) }; px < max.x; ++px)
			{
				for (int py{ static_cast<int>(min.y) }; py < max.y; ++py)
				{
					Vector2 pixel{ static_cast<float>(px), static_cast<float>(py) };

					const float w0{ (Vector2::Cross(v2 - v1, pixel - v1) / 2.f) / area };

					if (w0 < 0)
					{
						continue;
					}

					const float w1{ (Vector2::Cross(v0 - v2, pixel - v2) / 2.f) / area };

					if (w1 < 0)
					{
						continue;
					}

					const float w2{ (Vector2::Cross(v1 - v0, pixel - v0) / 2.f) / area };

					if (w2 < 0)
					{
						continue;
					}

					const int pixelIndex{ py * m_Width + px };
					const float depthValue{ vertex0.position.z * w0
											+ vertex1.position.z * w1
											+ vertex2.position.z * w2 };

					if (depthValue <= m_pDepthBufferPixels[pixelIndex])
					{
						m_pDepthBufferPixels[pixelIndex] = depthValue;
					}
					else continue;

					Vector2 interpolation{ vertex0.uv * w0 + vertex1.uv * w1 + vertex2.uv * w2 };
					
					ColorRGBA finalColor{ m_pVehicleDiffuseTexture->Sample(interpolation) };

					//Update Color in Buffer
					finalColor.MaxToOne();

					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
			}
		}
	}
}

void Renderer::W2_Part3() const
{
	std::vector<Mesh> meshes_world
	{
		Mesh
		{
			{
				Vertex{ { -3.f, 3.f, -2.f }, {}, { 0.f, 0.f } },
				Vertex{ { 0.f, 3.f, -2.f }, {}, { 0.5f, 0.f } },
				Vertex{ { 3.f, 3.f, -2.f }, {}, { 1.f, 0.f } },
				Vertex{ { -3.f, 0.f, -2.f }, {}, { 0.f, 0.5f } },
				Vertex{ { 0.f, 0.f, -2.f }, {}, { 0.5f, 0.5f } },
				Vertex{ { 3.f, 0.f, -2.f }, {}, { 1.f, 0.5f } },
				Vertex{ { -3.f, -3.f, -2.f }, {}, { 0.f, 1.f } },
				Vertex{ { 0.f, -3.f, -2.f }, {}, { 0.5f, 1.f } },
				Vertex{ { 3.f, -3.f, -2.f }, {}, { 1.f, 1.f } }
			},

			/*{
				3, 0, 1,	1, 4, 3,	4, 1, 2,
				2, 5, 4,	6, 3, 4,	4, 7, 6,
				7, 4, 5,	5, 8, 7
			},

			PrimitiveTopology::TriangeList*/

			{
				3, 0, 4, 1, 5, 2,
				2, 6,
				6, 3, 7, 4, 8, 5
			},

			PrimitiveTopology::TriangleStrip
		}
	};

	const int amountOfMeshes{ static_cast<int>(meshes_world.size()) };
	for (int meshIndex{}; meshIndex < amountOfMeshes; ++meshIndex)
	{
		Mesh mesh{ meshes_world[meshIndex] };

		int maxCount{};
		int increment{};

		if (mesh.primitiveTopology == PrimitiveTopology::TriangeList)
		{
			increment = 3;
			maxCount = static_cast<int>(meshes_world[meshIndex].indices.size());
		}
		else
		{
			increment = 1;
			maxCount = static_cast<int>(meshes_world[meshIndex].indices.size()) - 2;
		}

		std::vector<Vertex> transformed_vertices_world{};
		VertexTransformationFunction(mesh.vertices, transformed_vertices_world);

		for (int index{}; index < maxCount; index += increment)
		{
			if (   mesh.indices[index] == mesh.indices[index + 1]
				|| mesh.indices[index + 1] == mesh.indices[index + 2]
				|| mesh.indices[index + 2] == mesh.indices[index]     )
			{
				continue;
			}

			Vertex vertex0{ transformed_vertices_world[mesh.indices[index]] };
			Vertex vertex1{ transformed_vertices_world[mesh.indices[index + 1]] };
			Vertex vertex2{ transformed_vertices_world[mesh.indices[index + 2]] };

			const Vector2 v0{ vertex0.position.x, vertex0.position.y };
			const Vector2 v1{ vertex1.position.x, vertex1.position.y };
			const Vector2 v2{ vertex2.position.x, vertex2.position.y };

			float w0{};
			float w1{};
			float w2{};

			Vector2 v1ToV2{ v2 - v1 };
			Vector2 v2ToV0{ v0 - v2 };
			Vector2 v0ToV1{ v1 - v0 };

			const float area{ Vector2::Cross(v0ToV1, v2 - v0) / 2.f };

			Vector2 min = { std::min(v0.x, v1.x), std::min(v0.y, v1.y) };
			min.x = std::min(min.x, v2.x);
			min.x = std::max(min.x, 0.f);
			min.y = std::min(min.y, v2.y);
			min.y = std::max(min.y, 0.f);

			Vector2 max = { std::max(v0.x, v1.x), std::max(v0.y, v1.y) };
			max.x = std::max(max.x, v2.x);
			max.x = std::min(max.x, static_cast<float>(m_Width));
			max.y = std::max(max.y, v2.y);
			max.y = std::min(max.y, static_cast<float>(m_Height));

			for (int px{ static_cast<int>(min.x) }; px < max.x; ++px)
			{
				for (int py{ static_cast<int>(min.y) }; py < max.y; ++py)
				{
					Vector2 pixel = { static_cast<float>(px), static_cast<float>(py) };

					if (mesh.primitiveTopology == PrimitiveTopology::TriangleStrip && index & 0x01)
					{
						std::swap(vertex1, vertex2);
					}
					
					w0 = Vector2::Cross(v1ToV2, pixel - v1);

					if (w0 < 0)
					{
						continue;
					}

					w1 = Vector2::Cross(v2ToV0, pixel - v2);

					if (w1 < 0)
					{
						continue;
					}

					w2 = Vector2::Cross(v0ToV1, pixel - v0);

					if (w2 < 0)
					{
						continue;
					}

					w0 = (w0 / 2.f) / area;
					w1 = (w1 / 2.f) / area;
					w2 = (w2 / 2.f) / area;

					float zInterpolated
					{ 
						1.f / (  (1.f * w0) / vertex0.position.z
							   + (1.f * w1) / vertex1.position.z
							   + (1.f * w2) / vertex2.position.z )
					};

					int pixelIndex{ py * m_Width + px };

					if (zInterpolated <= m_pDepthBufferPixels[pixelIndex])
					{
						m_pDepthBufferPixels[pixelIndex] = zInterpolated;
					}
					else continue;

					Vector2 interpolatedUV
					{ 
						zInterpolated * (  (vertex0.uv * w0) / vertex0.position.z
										 + (vertex1.uv * w1) / vertex1.position.z
										 + (vertex2.uv * w2) / vertex2.position.z )
					};

					ColorRGBA finalColor = m_pVehicleDiffuseTexture->Sample(interpolatedUV);

					//Update Color in Buffer
					finalColor.MaxToOne();

					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
			}
		}
	}
}

void Renderer::W3_Part1()
{
	std::vector<Mesh> meshes_world
	{
		Mesh
		{
			{
				Vertex{ { -3.f, 3.f, -2.f }, {}, { 0.f, 0.f } },
				Vertex{ { 0.f, 3.f, -2.f }, {}, { 0.5f, 0.f } },
				Vertex{ { 3.f, 3.f, -2.f }, {}, { 1.f, 0.f } },
				Vertex{ { -3.f, 0.f, -2.f }, {}, { 0.f, 0.5f } },
				Vertex{ { 0.f, 0.f, -2.f }, {}, { 0.5f, 0.5f } },
				Vertex{ { 3.f, 0.f, -2.f }, {}, { 1.f, 0.5f } },
				Vertex{ { -3.f, -3.f, -2.f }, {}, { 0.f, 1.f } },
				Vertex{ { 0.f, -3.f, -2.f }, {}, { 0.5f, 1.f } },
				Vertex{ { 3.f, -3.f, -2.f }, {}, { 1.f, 1.f } }
			},

			/*{
				3, 0, 1,	1, 4, 3,	4, 1, 2,
				2, 5, 4,	6, 3, 4,	4, 7, 6,
				7, 4, 5,	5, 8, 7
			},

			PrimitiveTopology::TriangeList*/

			{
				3, 0, 4, 1, 5, 2,
				2, 6,
				6, 3, 7, 4, 8, 5
			},

			PrimitiveTopology::TriangleStrip
		}
	};

	VertexTransformationFunction(meshes_world);

	const int amountOfMeshes{ static_cast<int>(meshes_world.size()) };
	for (Mesh& mesh : meshes_world)
	{
		for (Vertex_Out& vertex : mesh.vertices_out)
		{
			vertex.position.x = 0.5f * (vertex.position.x + 1.f) * m_Width;
			vertex.position.y = 0.5f * (1.f - vertex.position.y) * m_Height;
		}

		int maxCount{};
		int increment{};

		if (mesh.primitiveTopology == PrimitiveTopology::TriangeList)
		{
			increment = 3;
			maxCount = static_cast<int>(mesh.indices.size());
		}
		else
		{
			increment = 1;
			maxCount = static_cast<int>(mesh.indices.size()) - 2;
		}

		for (int index{}; index < maxCount; index += increment)
		{
			if (mesh.indices[index] == mesh.indices[index + 1]
				|| mesh.indices[index + 1] == mesh.indices[index + 2]
				|| mesh.indices[index + 2] == mesh.indices[index])
			{
				continue;
			}

			Vertex_Out& vertex0{ mesh.vertices_out[mesh.indices[index]] };
			Vertex_Out& vertex1{ mesh.vertices_out[mesh.indices[index + 1]] };
			Vertex_Out& vertex2{ mesh.vertices_out[mesh.indices[index + 2]] };

			if (vertex0.position.z < 0.f || vertex0.position.z > 1.f
				|| vertex1.position.z < 0.f || vertex1.position.z > 1.f
				|| vertex2.position.z < 0.f || vertex2.position.z > 1.f) continue;

			const Vector2 v0{ vertex0.position.x, vertex0.position.y };
			const Vector2 v1{ vertex1.position.x, vertex1.position.y };
			const Vector2 v2{ vertex2.position.x, vertex2.position.y };

			float w0{};
			float w1{};
			float w2{};

			Vector2 v1ToV2{ v2 - v1 };
			Vector2 v2ToV0{ v0 - v2 };
			Vector2 v0ToV1{ v1 - v0 };

			const float area{ Vector2::Cross(v0ToV1, v2 - v0) / 2.f };

			Vector2 min = { std::min(v0.x, v1.x), std::min(v0.y, v1.y) };
			min.x = std::min(min.x, v2.x);
			min.x = std::max(min.x, 0.f);
			min.y = std::min(min.y, v2.y);
			min.y = std::max(min.y, 0.f);

			Vector2 max = { std::max(v0.x, v1.x), std::max(v0.y, v1.y) };
			max.x = std::max(max.x, v2.x);
			max.x = std::min(max.x, static_cast<float>(m_Width));
			max.y = std::max(max.y, v2.y);
			max.y = std::min(max.y, static_cast<float>(m_Height));

			for (int px{ static_cast<int>(min.x) }; px < max.x; ++px)
			{
				for (int py{ static_cast<int>(min.y) }; py < max.y; ++py)
				{
					Vector2 pixel = { static_cast<float>(px), static_cast<float>(py) };

					if (mesh.primitiveTopology == PrimitiveTopology::TriangleStrip && index & 0x01)
					{
						w0 = Vector2::Cross(v1ToV2, pixel - v1);

						if (w0 > 0)
						{
							continue;
						}

						w1 = Vector2::Cross(v2ToV0, pixel - v2);

						if (w1 > 0)
						{
							continue;
						}

						w2 = Vector2::Cross(v0ToV1, pixel - v0);

						if (w2 > 0)
						{
							continue;
						}

						w0 = (w0 / 2.f) / (-1 * area);
						w1 = (w1 / 2.f) / (-1 * area);
						w2 = (w2 / 2.f) / (-1 * area);
					}
					else
					{
						w0 = Vector2::Cross(v1ToV2, pixel - v1);

						if (w0 < 0)
						{
							continue;
						}

						w1 = Vector2::Cross(v2ToV0, pixel - v2);

						if (w1 < 0)
						{
							continue;
						}

						w2 = Vector2::Cross(v0ToV1, pixel - v0);

						if (w2 < 0)
						{
							continue;
						}

						w0 = (w0 / 2.f) / area;
						w1 = (w1 / 2.f) / area;
						w2 = (w2 / 2.f) / area;
					}

					float depthInterpolated
					{
						1.f / ((1.f * w0) / vertex0.position.z
							   + (1.f * w1) / vertex1.position.z
							   + (1.f * w2) / vertex2.position.z)
					};

					int pixelIndex{ py * m_Width + px };

					if (depthInterpolated <= m_pDepthBufferPixels[pixelIndex])
					{
						m_pDepthBufferPixels[pixelIndex] = depthInterpolated;
					}
					else continue;

					const float wInterpolated
					{
						1.f / ((1.f * w0) * vertex0.position.w
							   + (1.f * w1) * vertex1.position.w
							   + (1.f * w2) * vertex2.position.w)
					};

					Vector2 interpolatedUV
					{
						wInterpolated * ((vertex0.uv * w0) * vertex0.position.w
										 + (vertex1.uv * w1) * vertex1.position.w
										 + (vertex2.uv * w2) * vertex2.position.w)
					};

					ColorRGBA finalColor = m_pVehicleDiffuseTexture->Sample(interpolatedUV);

					//Update Color in Buffer
					finalColor.MaxToOne();

					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
			}
		}
	}
}

void Renderer::W3_Part2()
{
	VertexTransformationFunction(m_MeshesWorld);

	const int amountOfMeshes{ static_cast<int>(m_MeshesWorld.size()) };
	for (Mesh& mesh : m_MeshesWorld)
	{
		for (Vertex_Out& vertex : mesh.vertices_out)
		{
			vertex.position.x = 0.5f * (vertex.position.x + 1.f) * m_Width;
			vertex.position.y = 0.5f * (1.f - vertex.position.y) * m_Height;
		}

		int maxCount{};
		int increment{};

		if (mesh.primitiveTopology == PrimitiveTopology::TriangeList)
		{
			increment = 3;
			maxCount = static_cast<int>(mesh.indices.size());
		}
		else
		{
			increment = 1;
			maxCount = static_cast<int>(mesh.indices.size()) - 2;
		}

		for (int index{}; index < maxCount; index += increment)
		{
			if (mesh.indices[index] == mesh.indices[index + 1]
				|| mesh.indices[index + 1] == mesh.indices[index + 2]
				|| mesh.indices[index + 2] == mesh.indices[index])
			{
				continue;
			}

			Vertex_Out& vertex0{ mesh.vertices_out[mesh.indices[index]] };
			Vertex_Out& vertex1{ mesh.vertices_out[mesh.indices[index + 1]] };
			Vertex_Out& vertex2{ mesh.vertices_out[mesh.indices[index + 2]] };

			if (vertex0.position.z < 0.f || vertex0.position.z > 1.f
				|| vertex1.position.z < 0.f || vertex1.position.z > 1.f
				|| vertex2.position.z < 0.f || vertex2.position.z > 1.f) continue;

			const Vector2 v0{ vertex0.position.x, vertex0.position.y };
			const Vector2 v1{ vertex1.position.x, vertex1.position.y };
			const Vector2 v2{ vertex2.position.x, vertex2.position.y };

			float w0{};
			float w1{};
			float w2{};

			Vector2 v1ToV2{ v2 - v1 };
			Vector2 v2ToV0{ v0 - v2 };
			Vector2 v0ToV1{ v1 - v0 };

			const float area{ Vector2::Cross(v0ToV1, v2 - v0) / 2.f };

			Vector2 min = { std::min(v0.x, v1.x), std::min(v0.y, v1.y) };
			min.x = std::min(min.x, v2.x);
			min.x = std::max(min.x, 0.f);
			min.y = std::min(min.y, v2.y);
			min.y = std::max(min.y, 0.f);

			Vector2 max = { std::max(v0.x, v1.x), std::max(v0.y, v1.y) };
			max.x = std::max(max.x, v2.x);
			max.x = std::min(max.x, static_cast<float>(m_Width));
			max.y = std::max(max.y, v2.y);
			max.y = std::min(max.y, static_cast<float>(m_Height));

			for (int px{ static_cast<int>(min.x) }; px < max.x; ++px)
			{
				for (int py{ static_cast<int>(min.y) }; py < max.y; ++py)
				{
					Vector2 pixel = { static_cast<float>(px), static_cast<float>(py) };

					if (mesh.primitiveTopology == PrimitiveTopology::TriangleStrip && index & 0x01)
					{
						w0 = Vector2::Cross(v1ToV2, pixel - v1);

						if (w0 > 0)
						{
							continue;
						}

						w1 = Vector2::Cross(v2ToV0, pixel - v2);

						if (w1 > 0)
						{
							continue;
						}

						w2 = Vector2::Cross(v0ToV1, pixel - v0);

						if (w2 > 0)
						{
							continue;
						}

						w0 = (w0 / 2.f) / (-1 * area);
						w1 = (w1 / 2.f) / (-1 * area);
						w2 = (w2 / 2.f) / (-1 * area);
					}
					else
					{
						w0 = Vector2::Cross(v1ToV2, pixel - v1);

						if (w0 < 0)
						{
							continue;
						}

						w1 = Vector2::Cross(v2ToV0, pixel - v2);

						if (w1 < 0)
						{
							continue;
						}

						w2 = Vector2::Cross(v0ToV1, pixel - v0);

						if (w2 < 0)
						{
							continue;
						}

						w0 = (w0 / 2.f) / area;
						w1 = (w1 / 2.f) / area;
						w2 = (w2 / 2.f) / area;
					}

					float depthInterpolated
					{
						1.f / ((1.f * w0) / vertex0.position.z
							   + (1.f * w1) / vertex1.position.z
							   + (1.f * w2) / vertex2.position.z)
					};

					int pixelIndex{ py * m_Width + px };

					if (depthInterpolated <= m_pDepthBufferPixels[pixelIndex])
					{
						m_pDepthBufferPixels[pixelIndex] = depthInterpolated;
					}
					else continue;

					float interpolatedCameraSpaceZ{};
					ColorRGBA finalColor{};
					Vector2 interpolatedUV{};

					switch (m_RenderMode)
					{
					case dae::Renderer::RenderMode::combined:
						interpolatedCameraSpaceZ =
						{
							1.f / ((1.f * w0) * vertex0.position.w
								   + (1.f * w1) * vertex1.position.w
								   + (1.f * w2) * vertex2.position.w)
						};

						interpolatedUV =
						{
							interpolatedCameraSpaceZ * ((vertex0.uv * w0) * vertex0.position.w
											 + (vertex1.uv * w1) * vertex1.position.w
											 + (vertex2.uv * w2) * vertex2.position.w)
						};

						finalColor = m_pVehicleDiffuseTexture->Sample(interpolatedUV);
						break;

					case dae::Renderer::RenderMode::observedArea:
						const float remappedDepth{ Remap(depthInterpolated, 0.995f) };
						finalColor = { remappedDepth, remappedDepth, remappedDepth };
						break;
					}

					//Update Color in Buffer
					finalColor.MaxToOne();

					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
			}
		}
	}
}

void Renderer::W4_Part1()
{
	VertexTransformationFunction(m_MeshesWorld);
	random = 0;
	int number{};
	const int amountOfMeshes{ static_cast<int>(m_MeshesWorld.size()) };
	for (Mesh& mesh : m_MeshesWorld)
	{
		for (Vertex_Out& vertex : mesh.vertices_out)
		{
			vertex.position.x = 0.5f * (vertex.position.x + 1.f) * m_Width;
			vertex.position.y = 0.5f * (1.f - vertex.position.y) * m_Height;
		}

		int maxCount{};
		int increment{};

		if (mesh.primitiveTopology == PrimitiveTopology::TriangeList)
		{
			increment = 3;
			maxCount = static_cast<int>(mesh.indices.size());
		}
		else
		{
			increment = 1;
			maxCount = static_cast<int>(mesh.indices.size()) - 2;
		}

		for (int index{}; index < maxCount; index += increment)
		{
			if (mesh.indices[index] == mesh.indices[index + 1]
				|| mesh.indices[index + 1] == mesh.indices[index + 2]
				|| mesh.indices[index + 2] == mesh.indices[index])
			{
				continue;
			}

			Vertex_Out& vertex0{ mesh.vertices_out[mesh.indices[index]] };
			Vertex_Out& vertex1{ mesh.vertices_out[mesh.indices[index + 1]] };
			Vertex_Out& vertex2{ mesh.vertices_out[mesh.indices[index + 2]] };

			if (vertex0.position.z < 0.f || vertex0.position.z > 1.f
				|| vertex1.position.z < 0.f || vertex1.position.z > 1.f
				|| vertex2.position.z < 0.f || vertex2.position.z > 1.f) continue;

			const Vector2 v0{ vertex0.position.x, vertex0.position.y };
			const Vector2 v1{ vertex1.position.x, vertex1.position.y };
			const Vector2 v2{ vertex2.position.x, vertex2.position.y };

			Vector2 v1ToV2{ v2 - v1 };
			Vector2 v2ToV0{ v0 - v2 };
			Vector2 v0ToV1{ v1 - v0 };

			const float area{ Vector2::Cross(v0ToV1, v2 - v0) / 2.f };

			if (mesh.cullMode == CullMode::FrontFaceCulling && area > 0.f)
			{
				++random;
				continue;
			}

			if (mesh.cullMode == CullMode::BackFaceCulling && area < 0.f)
				continue;

			float w0{};
			float w1{};
			float w2{};

			Vector2 min{};
			Vector2 max{};
			CalculateBoundingBox(v0, v1, v2, min, max);

			for (int px{ static_cast<int>(min.x) }; px < max.x; ++px)
			{
				for (int py{ static_cast<int>(min.y) }; py < max.y; ++py)
				{
					Vector2 pixelPos = { static_cast<float>(px), static_cast<float>(py) };

					if (!IsPixelInTriange(v0, v1, v2, pixelPos))
						continue;

					if (mesh.primitiveTopology == PrimitiveTopology::TriangleStrip && index & 0x01)
					{
						w0 = (Vector2::Cross(v1ToV2, pixelPos - v1) / 2.f) / (-1 * area);
						w1 = (Vector2::Cross(v2ToV0, pixelPos - v2) / 2.f) / (-1 * area);
						w2 = (Vector2::Cross(v0ToV1, pixelPos - v0) / 2.f) / (-1 * area);
					}
					else if(mesh.primitiveTopology == PrimitiveTopology::TriangeList)
					{
						w0 = (Vector2::Cross(v1ToV2, pixelPos - v1) / 2.f) / area;
						w1 = (Vector2::Cross(v2ToV0, pixelPos - v2) / 2.f) / area;
						w2 = (Vector2::Cross(v0ToV1, pixelPos - v0) / 2.f) / area;
					}

					float depthInterpolated
					{
						1.f / ((1.f * w0) / vertex0.position.z
							   + (1.f * w1) / vertex1.position.z
							   + (1.f * w2) / vertex2.position.z)
					};

					int pixelIndex{ py * m_Width + px };

					if (number == 1)
					{
						if (depthInterpolated >= m_pDepthBufferPixels[pixelIndex])
							continue;
					}
					
					if (number == 0)
					{
						if (depthInterpolated <= m_pDepthBufferPixels[pixelIndex])
						{
							m_pDepthBufferPixels[pixelIndex] = depthInterpolated;
						}
						else continue;
					}
					
					float interpolatedCameraSpaceZ{};
					ColorRGBA finalColor{};
					Vector2 interpolatedUV{};
					Vertex_Out pixel{};
					
					interpolatedCameraSpaceZ =
					{
						1.f / (  w0 * vertex0.position.w
							   + w1 * vertex1.position.w
							   + w2 * vertex2.position.w)
					};

					pixel.uv =
					{
						interpolatedCameraSpaceZ *
						(vertex0.uv * w0 * vertex0.position.w
						+ vertex1.uv * w1 * vertex1.position.w
						+ vertex2.uv * w2 * vertex2.position.w)
					};

					pixel.position =
					{
						pixelPos.x,
						pixelPos.y,
						depthInterpolated,
						interpolatedCameraSpaceZ
					};

					pixel.normal =
					{
						Vector3{vertex0.normal * w0 * vertex0.position.w
								+ vertex1.normal * w1 * vertex1.position.w
								+ vertex2.normal * w2 * vertex2.position.w}.Normalized()
					};

					pixel.tangent =
					{
						Vector3{vertex0.tangent * w0 * vertex0.position.w
								+ vertex1.tangent * w1 * vertex1.position.w
								+ vertex2.tangent * w2 * vertex2.position.w}.Normalized()
					};

					pixel.viewDirection =
					{
						Vector3{vertex0.viewDirection * w0 * vertex0.position.w
								+ vertex1.viewDirection * w1 * vertex1.position.w
								+ vertex2.viewDirection * w2 * vertex2.position.w}
					};

					finalColor = ShadePixel(pixel, number);

					if (number == 1)
					{
						Uint8 rValue{}, gValue{}, bValue{};
						SDL_GetRGB(m_pBackBufferPixels[static_cast<int>(pixel.position.x) + (static_cast<int>(pixel.position.y) * m_Width)], m_pBackBuffer->format, &rValue, &gValue, &bValue);

						finalColor.a = std::min(1.f, finalColor.a);

						finalColor =
						{
							finalColor.a * finalColor.r + (1.f - finalColor.a) * (rValue / 255.f),
							finalColor.a * finalColor.g + (1.f - finalColor.a) * (gValue / 255.f),
							finalColor.a * finalColor.b + (1.f - finalColor.a) * (bValue / 255.f)
						};
					}

					//Update Color in Buffer
					finalColor.MaxToOne();
					
					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
			}
		}
		++number;
	}
	std::cout << random << '\n';
}

bool Renderer::IsPixelInTriange(const Vector2& v0, const Vector2& v1, const Vector2& v2, const Vector2& pixelPos) const
{
	float cross1{ Vector2::Cross(v2 - v1, pixelPos - v1) };
	bool cross1IsPositive{};

	if (cross1 > 0)
	{
		cross1IsPositive = true;
	}

	float cross2{ Vector2::Cross(v0 - v2, pixelPos - v2) };
	bool cross2IsPositive{};

	if (cross2 > 0)
	{
		cross2IsPositive = true;
	}

	float cross3{ Vector2::Cross(v1 - v0, pixelPos - v0) };
	bool cross3IsPositive{};

	if (cross3 > 0)
	{
		cross3IsPositive = true;
	}

	if (	cross1IsPositive && cross2IsPositive && cross3IsPositive
		|| !cross1IsPositive && !cross2IsPositive && !cross3IsPositive)
		return true;

	return false;
}

void Renderer::CalculateBoundingBox(const Vector2& v0, const Vector2& v1, const Vector2& v2, Vector2& min, Vector2& max)
{
	min = { std::min(v0.x, v1.x), std::min(v0.y, v1.y) };
	min.x = std::min(min.x, v2.x);
	min.x = std::max(min.x, 0.f);
	min.y = std::min(min.y, v2.y);
	min.y = std::max(min.y, 0.f);

	max = { std::max(v0.x, v1.x), std::max(v0.y, v1.y) };
	max.x = std::max(max.x, v2.x);
	max.x = std::min(max.x, static_cast<float>(m_Width));
	max.y = std::max(max.y, v2.y);
	max.y = std::min(max.y, static_cast<float>(m_Height));
}

void RenderTriangle(const Vector2& v0, const Vector2& v1, const Vector2& v2, Vector2& min, Vector2& max)
{

}

ColorRGBA Renderer::ShadePixel(const Vertex_Out& vertex, int number) const
{
	if (m_VisualizeDepthBuffer)
	{
		const float value{ Remap(vertex.position.z, 0.995f) };
		return {value, value, value};
	}

	float observedArea{};
	const Vector3 lightDirection{ 0.577f, -0.577f, 0.577f };
	constexpr float lightIntensity{ 7.f };
	constexpr float shininess{ 25.f };
	Vector3 normal{ vertex.normal };

	if (m_UseNormalMap)
	{
		ColorRGBA sampledNormal{ m_pNormalMap->Sample(vertex.uv) };

		//remap the normal values to range [-1, 1]
		sampledNormal.r = 2.f * sampledNormal.r - 1.f;
		sampledNormal.g = 2.f * sampledNormal.g - 1.f;
		sampledNormal.b = 2.f * sampledNormal.b - 1.f;
		sampledNormal.a = 2.f * sampledNormal.a - 1.f;

		Vector3 binormal{ Vector3::Cross(vertex.normal, vertex.tangent) };
		Matrix tangentSpaceAxis{ vertex.tangent, binormal, vertex.normal, Vector3::Zero };

		//transform the sampledNormal to tangent space
		normal = { sampledNormal.r, sampledNormal.g, sampledNormal.b };
		normal = tangentSpaceAxis.TransformVector(normal).Normalized();
	}
	
	observedArea = std::max(0.f, Vector3::Dot(normal, -lightDirection));

	switch (m_RenderMode)
	{
		case dae::Renderer::RenderMode::combined:
		{
			constexpr ColorRGBA ambient{ 0.025f, 0.025f, 0.025f };

			//lambert diffuse
			ColorRGBA diffuse{};
			if (number == 0)
			{
				diffuse = lightIntensity * m_pVehicleDiffuseTexture->Sample(vertex.uv) / PI;
			}
			else
			{
				diffuse = lightIntensity * m_pCombustionEffectDiffuseMap->Sample(vertex.uv) / PI;
				return diffuse;
			}

			//specular phong
			ColorRGBA specular{ m_pSpecularMap->Sample(vertex.uv) * powf(std::max(Vector3::Dot(2.f * std::max(Vector3::Dot(normal, -lightDirection), 0.f) * normal - -lightDirection, vertex.viewDirection), 0.f), shininess * m_pGlossinessMap->Sample(vertex.uv).r) }; //glossinessMap is greyscale so all channels have the same value

			return (diffuse + specular + ambient) * observedArea;
		}
	
		case dae::Renderer::RenderMode::observedArea:
			return { observedArea, observedArea, observedArea };

		case dae::Renderer::RenderMode::diffuse:
		{
			ColorRGBA diffuse{ lightIntensity * m_pVehicleDiffuseTexture->Sample(vertex.uv) / PI };
			return diffuse * observedArea;
		}

		case dae::Renderer::RenderMode::specular:
		{
			ColorRGBA specular{ m_pSpecularMap->Sample(vertex.uv) * powf(std::max(Vector3::Dot(2.f * std::max(Vector3::Dot(normal, -lightDirection), 0.f) * normal - -lightDirection, vertex.viewDirection), 0.f), shininess * m_pGlossinessMap->Sample(vertex.uv).r) }; //glossinessMap is greyscale so all channels have the same value
			return specular * observedArea;
		}
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}

void Renderer::ChangeRenderMode()
{
	switch (m_RenderMode)
	{
	case RenderMode::combined:
		m_RenderMode = RenderMode::observedArea;
		break;

	case RenderMode::observedArea:
		m_RenderMode = RenderMode::diffuse;
		break;

	case RenderMode::diffuse:
		m_RenderMode = RenderMode::specular;
		break;

	case RenderMode::specular:
		m_RenderMode = RenderMode::combined;
	}
}

void Renderer::SetIsRotating(bool isRotating)
{
	m_IsRotating = isRotating;
}

bool Renderer::GetIsRotating() const
{
	return m_IsRotating;
}

void Renderer::SetUseNormalMap(bool useNormalMap)
{
	m_UseNormalMap = useNormalMap;
}

bool Renderer::GetUseNormalMap() const
{
	return m_UseNormalMap;
}

void Renderer::SetVisualizeDepthBuffer(bool visualizeDepthBuffer)
{
	m_VisualizeDepthBuffer = visualizeDepthBuffer;
}

bool Renderer::GetVisualizeDepthBuffer() const
{
	return m_VisualizeDepthBuffer;
}