//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"

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

	//m_pDepthBufferPixels = new float[m_Width * m_Height];

	//Initialize Camera
	m_Camera.Initialize(60.f, { .0f,.0f,-10.f });
}

Renderer::~Renderer()
{
	//delete[] m_pDepthBufferPixels;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
	SDL_FillRect(m_pBackBuffer, NULL, 0x000000);
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	//RENDER LOGIC
	//W1_Part1();
	//W1_Part2();
	//W1_Part3();
	W1_Part4();
	
	//for (int px{}; px < m_Width; ++px)
	//{
	//	for (int py{}; py < m_Height; ++py)
	//	{
	//		float gradient = px / static_cast<float>(m_Width);
	//		gradient += py / static_cast<float>(m_Width);
	//		gradient /= 2.0f;

	//		ColorRGB finalColor{ gradient, gradient, gradient };

	//		//Update Color in Buffer
	//		finalColor.MaxToOne();

	//		m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
	//			static_cast<uint8_t>(finalColor.r * 255),
	//			static_cast<uint8_t>(finalColor.g * 255),
	//			static_cast<uint8_t>(finalColor.b * 255));
	//	}
	//}

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

		//add the vertex to vertices_out
		vertices_out.push_back(projectedVertex);
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

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			int amountOfTriangles{ static_cast<int>(transformed_vertices_world.size()) / 3 };
			for (int index{}; index < amountOfTriangles; ++index)
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

				ColorRGB finalColor{ transformed_vertices_world[index * 3].color * w0 + transformed_vertices_world[index * 3 + 1].color * w1 + transformed_vertices_world[index * 3 + 2].color * w2 };

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

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}