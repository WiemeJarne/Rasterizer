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
	//Lock BackBuffer
	SDL_LockSurface(m_pBackBuffer);

	//RENDER LOGIC
	//Part1();
	Part2();
	
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
	//Todo > W1 Projection Stage
}

void Renderer::Part1() const
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

void Renderer::Part2() const
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
	vertices_screenSpace[0].z = vertices_ndc[0].z;
	vertices_screenSpace[1].x = (vertices_ndc[1].x + 1) / 2.f * m_Width;
	vertices_screenSpace[1].y = (1 - vertices_ndc[1].y) / 2.f * m_Height;
	vertices_screenSpace[1].z = vertices_ndc[1].z;
	vertices_screenSpace[2].x = (vertices_ndc[2].x + 1) / 2.f * m_Width;
	vertices_screenSpace[2].y = (1 - vertices_ndc[2].y) / 2.f * m_Height;
	vertices_screenSpace[2].z = vertices_ndc[2].z;

	for (int px{}; px < m_Width; ++px)
	{
		for (int py{}; py < m_Height; ++py)
		{
			Vector3 pixel{ static_cast<float>(px), static_cast<float>(py), 1 };

			m_Camera.viewMatrix.TransformPoint(pixel);

			Vector3 projectedVertex{};
			projectedVertex.x = pixel.x / pixel.z;
			projectedVertex.y = pixel.y / pixel.z;
			projectedVertex.z = pixel.z;

			projectedVertex.x = projectedVertex.x / ((m_Width / m_Height) * m_Camera.fov);
			projectedVertex.y = projectedVertex.y / m_Camera.fov;

			Vector3 p0Pixel{ pixel - Vector3(vertices_ndc[0].x, vertices_ndc[0].y, vertices_ndc[0].z) };
			Vector3 edge1{ Vector3(vertices_ndc[1].x, vertices_ndc[1].y, vertices_ndc[1].z) - Vector3(vertices_ndc[0].x, vertices_ndc[0].y, vertices_ndc[0].z) };

			Vector3 p1ToPixel{ pixel - Vector3(vertices_ndc[1].x, vertices_ndc[1].y, vertices_ndc[1].z) };
			Vector3 edge2{ Vector3(vertices_ndc[2].x, vertices_ndc[2].y, vertices_ndc[2].z) - Vector3(vertices_ndc[1].x, vertices_ndc[1].y, vertices_ndc[1].z) };

			Vector3 p2ToPixel{ pixel - Vector3(vertices_ndc[2].x, vertices_ndc[2].y, vertices_ndc[2].z) };
			Vector3 edge3{ Vector3(vertices_ndc[0].x, vertices_ndc[0].y, vertices_ndc[0].z) - Vector3(vertices_ndc[2].x, vertices_ndc[2].y, vertices_ndc[2].z) };


			if (    Vector3::Cross(edge1, p0Pixel).Magnitude() > 0
				 && Vector3::Cross(edge2, p1ToPixel).Magnitude() > 0
				 && Vector3::Cross(edge3, p2ToPixel).Magnitude() > 0 )
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

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}