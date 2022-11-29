#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"
#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
	struct Mesh;
	struct Vertex;
	class Timer;
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(Timer* pTimer);
		void Render();

		bool SaveBufferToImage() const;

		void ChangeRenderMode();

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		Camera m_Camera{};

		int m_Width{};
		int m_Height{};

		Texture* m_pTexture{};

		std::vector<Mesh> m_MeshesWorld{};

		enum class RenderMode
		{
			color,
			depth
		};

		RenderMode m_RenderMode{ RenderMode::color };

		//Function that transforms the vertices from the mesh from World space to Screen space
		void VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const;
		void VertexTransformationFunction(std::vector<Mesh>& meshes_world);
		void W1_Part1() const;
		void W1_Part2() const;
		void W1_Part3() const;
		void W1_Part4() const;
		void W1_Part5() const;

		void W2_Part1() const;
		void W2_Part2() const;
		void W2_Part3() const;

		void W3_Part1();
		void W3_Part2();

		void W4_Part1();

		ColorRGB PixelShading(const Vertex_Out& vertex) const;
	};
}