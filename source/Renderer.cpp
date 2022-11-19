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

	m_pTexture = Texture::LoadFromFile("Resources/uv_grid_2.png");

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
	delete m_pTexture;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
	Uint8 redValue{ 100 };
	Uint8 greenValue{ 100 };
	Uint8 blueValue{ 100 };
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
	W2_Part3();

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
				ColorRGB finalColor{ 1, 1, 1 };

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

			ColorRGB finalColor{ 1, 1, 1 };

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

			ColorRGB finalColor{ transformed_vertices_world[0].color * w0 + transformed_vertices_world[1].color * w1 + transformed_vertices_world[2].color * w2 };

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

	const int amountOfPixels{ m_Width * m_Height };
	for (int index{}; index < amountOfPixels; ++index)
	{
		m_pDepthBufferPixels[index] = INFINITY;
	}
	
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

				ColorRGB finalColor
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

	const int amountOfPixels{ m_Width * m_Height };
	for (int index{}; index < amountOfPixels; ++index)
	{
		m_pDepthBufferPixels[index] = INFINITY;
	}

	const int amountOfTriangles{ static_cast<int>(transformed_vertices_world.size()) / 3 };
	for (int index{}; index < amountOfTriangles; ++index)
	{
		const Vertex& vertex0{ transformed_vertices_world[index * 3] };
		const Vertex& vertex1{ transformed_vertices_world[index * 3 + 1] };
		const Vertex& vertex2{ transformed_vertices_world[index * 3 + 2] };

		TriangleBoundingBox boudingBox{};
		boudingBox.screenWith = m_Width;
		boudingBox.screenHeight = m_Height;
		boudingBox.Grow({ vertex0.position.x, vertex0.position.y });
		boudingBox.Grow({ vertex1.position.x, vertex1.position.y });
		boudingBox.Grow({ vertex2.position.x, vertex2.position.y });

		Vector2 v0{ vertex0.position.x, vertex0.position.y };
		Vector2 v1{ vertex1.position.x, vertex1.position.y };
		Vector2 v2{ vertex2.position.x, vertex2.position.y };

		float area{ Vector2::Cross(v1 - v0, v2 - v0) / 2.f };

		Vector2 v1ToV2{ v2 - v1 };
		Vector2 v2ToV0{ v0 - v2 };
		Vector2 v0ToV1{ v1 - v0 };

		Vector2 pixel{};
		int pixelIndex{};
		float depthValue{};
		ColorRGB finalColor{};
		float w0{};
		float w1{};
		float w2{};

		for (int px{ static_cast<int>(boudingBox.min.x) }; px < boudingBox.max.x; ++px)
		{
			for (int py{ static_cast<int>(boudingBox.min.y) }; py < boudingBox.max.y; ++py)
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
				}
				else continue;

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

	const int amountOfPixels{ m_Width * m_Height };
	for (int index{}; index < amountOfPixels; ++index)
	{
		m_pDepthBufferPixels[index] = INFINITY;
	}

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
			amountOfTriangles = static_cast<int>(meshes_world[meshIndex].indices.size()) / 3;
			amountOfTriangles *= 2;
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
					++amountOfTriangles; //no triangle has been rendered so increase amount of triangles otherwise the bottom halve of the mesh won't be rendered
					continue;
				}
			}

			const Vertex& vertex0{ transformed_vertices_world[mesh.indices[vertex0Index]] };
			const Vertex& vertex1{ transformed_vertices_world[mesh.indices[vertex1Index]] };
			const Vertex& vertex2{ transformed_vertices_world[mesh.indices[vertex2Index]] };

			Vector2 v0{ vertex0.position.x, vertex0.position.y };
			Vector2 v1{ vertex1.position.x, vertex1.position.y };
			Vector2 v2{ vertex2.position.x, vertex2.position.y };

			TriangleBoundingBox boudingBox{};
			boudingBox.screenWith = m_Width;
			boudingBox.screenHeight = m_Height;
			boudingBox.Grow(v0);
			boudingBox.Grow(v1);
			boudingBox.Grow(v2);

			for (int px{ static_cast<int>(boudingBox.min.x) }; px < boudingBox.max.x; ++px)
			{
				for (int py{ static_cast<int>(boudingBox.min.y) }; py < boudingBox.max.y; ++py)
				{
					Vector2 pixel{ static_cast<float>(px), static_cast<float>(py) };

					const float area{ Vector2::Cross(v1 - v0, v2 - v0) / 2.f };
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

					ColorRGB finalColor
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

			{
				3, 0, 1,	1, 4, 3,	4, 1, 2,
				2, 5, 4,	6, 3, 4,	4, 7, 6,
				7, 4, 5,	5, 8, 7
			},

			PrimitiveTopology::TriangeList
		}
	};

	const int amountOfPixels{ m_Width * m_Height };
	for (int index{}; index < amountOfPixels; ++index)
	{
		m_pDepthBufferPixels[index] = INFINITY;
	}

	const int amountOfMeshes{ static_cast<int>(meshes_world.size()) };
	for (int meshIndex{}; meshIndex < amountOfMeshes; ++meshIndex)
	{
		Mesh mesh{ meshes_world[meshIndex] };

		int amountOfTriangles{};

		if (mesh.primitiveTopology == PrimitiveTopology::TriangeList)
		{
			amountOfTriangles = static_cast<int>(meshes_world[meshIndex].indices.size()) / 3;
		}
		else if (mesh.primitiveTopology == PrimitiveTopology::TriangleStrip)
		{
			amountOfTriangles = static_cast<int>(meshes_world[meshIndex].indices.size()) / 3;
			amountOfTriangles *= 2;
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
					++amountOfTriangles; //no triangle has been rendered so increase amount of triangles otherwise the bottom halve of the mesh won't be rendered
					continue;
				}
			}

			const Vertex& vertex0{ transformed_vertices_world[mesh.indices[vertex0Index]] };
			const Vertex& vertex1{ transformed_vertices_world[mesh.indices[vertex1Index]] };
			const Vertex& vertex2{ transformed_vertices_world[mesh.indices[vertex2Index]] };

			Vector2 v0{ vertex0.position.x, vertex0.position.y };
			Vector2 v1{ vertex1.position.x, vertex1.position.y };
			Vector2 v2{ vertex2.position.x, vertex2.position.y };

			TriangleBoundingBox boudingBox{};
			boudingBox.screenWith = m_Width;
			boudingBox.screenHeight = m_Height;
			boudingBox.Grow(v0);
			boudingBox.Grow(v1);
			boudingBox.Grow(v2);

			for (int px{ static_cast<int>(boudingBox.min.x) }; px < boudingBox.max.x; ++px)
			{
				for (int py{ static_cast<int>(boudingBox.min.y) }; py < boudingBox.max.y; ++py)
				{
					Vector2 pixel{ static_cast<float>(px), static_cast<float>(py) };

					const float area{ Vector2::Cross(v1 - v0, v2 - v0) / 2.f };
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
					
					ColorRGB finalColor{ m_pTexture->Sample(interpolation) };

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

			{
				3, 0, 1,	1, 4, 3,	4, 1, 2,
				2, 5, 4,	6, 3, 4,	4, 7, 6,
				7, 4, 5,	5, 8, 7
			},

			PrimitiveTopology::TriangeList
		}
	};

	const int amountOfPixels{ m_Width * m_Height };
	for (int index{}; index < amountOfPixels; ++index)
	{
		m_pDepthBufferPixels[index] = INFINITY;
	}

	const int amountOfMeshes{ static_cast<int>(meshes_world.size()) };
	for (int meshIndex{}; meshIndex < amountOfMeshes; ++meshIndex)
	{
		Mesh mesh{ meshes_world[meshIndex] };

		int amountOfTriangles{};

		if (mesh.primitiveTopology == PrimitiveTopology::TriangeList)
		{
			amountOfTriangles = static_cast<int>(meshes_world[meshIndex].indices.size()) / 3;
		}
		else if (mesh.primitiveTopology == PrimitiveTopology::TriangleStrip)
		{
			amountOfTriangles = static_cast<int>(meshes_world[meshIndex].indices.size()) / 3;
			amountOfTriangles *= 2;
		}
		else break;

		std::vector<Vertex> transformed_vertices_world{};
		VertexTransformationFunction(mesh.vertices, transformed_vertices_world);

		//the indices to get the index form the mesh.indices vector
		int vertex0Index{};
		int vertex1Index{};
		int vertex2Index{};

		for (int triangleIndex{}; triangleIndex < amountOfTriangles; ++triangleIndex)
		{
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
					++amountOfTriangles; //no triangle has been rendered so increase amount of triangles otherwise the bottom halve of the mesh won't be rendered
					continue;
				}
			}

			const Vertex& vertex0{ transformed_vertices_world[mesh.indices[vertex0Index]] };
			const Vertex& vertex1{ transformed_vertices_world[mesh.indices[vertex1Index]] };
			const Vertex& vertex2{ transformed_vertices_world[mesh.indices[vertex2Index]] };

			const Vector2 v0{ vertex0.position.x, vertex0.position.y };
			const Vector2 v1{ vertex1.position.x, vertex1.position.y };
			const Vector2 v2{ vertex2.position.x, vertex2.position.y };

			const float area{ Vector2::Cross(v1 - v0, v2 - v0) / 2.f };

			TriangleBoundingBox boudingBox{};
			boudingBox.screenWith = m_Width;
			boudingBox.screenHeight = m_Height;
			boudingBox.Grow(v0);
			boudingBox.Grow(v1);
			boudingBox.Grow(v2);

			Vector2 pixel{};
			float w0{};
			float w1{};
			float w2{};

			Vector2 v1ToV2{ v2 - v1 };
			Vector2 v2ToV0{ v0 - v2 };
			Vector2 v0ToV1{ v1 - v0 };

			float zInterpolated{};
			int pixelIndex{};
			Vector2 interpolatedUV{};
			ColorRGB finalColor{};

			for (int px{ static_cast<int>(boudingBox.min.x) }; px < boudingBox.max.x; ++px)
			{
				for (int py{ static_cast<int>(boudingBox.min.y) }; py < boudingBox.max.y; ++py)
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

					zInterpolated =
					{ 
						1.f / (  (1.f * w0) / vertex0.position.z
							   + (1.f * w1) / vertex1.position.z
							   + (1.f * w2) / vertex2.position.z )
					};

					pixelIndex = { py * m_Width + px };

					if (zInterpolated <= m_pDepthBufferPixels[pixelIndex])
					{
						m_pDepthBufferPixels[pixelIndex] = zInterpolated;
					}
					else continue;

					interpolatedUV =
					{ 
						zInterpolated * (  (vertex0.uv * w0) / vertex0.position.z
										 + (vertex1.uv * w1) / vertex1.position.z
										 + (vertex2.uv * w2) / vertex2.position.z )
					};

					finalColor = m_pTexture->Sample(interpolatedUV);

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

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}