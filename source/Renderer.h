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

		void SetIsRotating(bool isRotating);
		bool GetIsRotating() const;

		void SetUseNormalMap(bool useNormalMap);
		bool GetUseNormalMap() const;

		void SetVisualizeDepthBuffer(bool visualizeDepthBuffer);
		bool GetVisualizeDepthBuffer() const;

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		Camera m_Camera{};

		int m_Width{};
		int m_Height{};

		Texture* m_pCombustionEffectDiffuseMap{};
		Texture* m_pVehicleDiffuseTexture{};
		Texture* m_pNormalMap{};
		Texture* m_pSpecularMap{};
		Texture* m_pGlossinessMap{};

		std::vector<Mesh> m_MeshesWorld{};

		bool m_IsRotating{ true };
		bool m_UseNormalMap{ true };
		bool m_VisualizeDepthBuffer{ false };

		enum class RenderMode
		{
			combined,
			observedArea,
			diffuse, //(incl. observed area)
			specular //(incl. observed area)
		};

		RenderMode m_RenderMode{ RenderMode::combined };

		Uint8* m_pBufferRValue{ new Uint8() }, * m_pBufferGValue{ new Uint8() }, * m_pBufferBValue{ new Uint8() };

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

		ColorRGBA ShadePixel(const Vertex_Out& vertex, int number) const;
	};
}