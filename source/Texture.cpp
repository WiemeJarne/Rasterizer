#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>
#include <algorithm>
namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	Texture::~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(const std::string& path)
	{
		auto surface{ IMG_Load(path.c_str()) };
		return new Texture(surface);
	}

	ColorRGBA Texture::Sample(const Vector2& uv)
	{
		const Vector2 scaledUV{ std::clamp(uv.x, 0.f, 1.f) * m_pSurface->w, std::clamp(uv.y, 0.f, 1.f) * m_pSurface->h};

		Uint32 pixel{ m_pSurfacePixels[static_cast<int>(scaledUV.y) * m_pSurface->h + static_cast<int>(scaledUV.x)] };

		Uint8 rValue{}, gValue{}, bValue{}, alphaValue{};
		SDL_GetRGBA(pixel, m_pSurface->format, &rValue, &gValue, &bValue, &alphaValue);

		return { rValue / 255.f, gValue / 255.f, bValue / 255.f, alphaValue / 255.f };
	}
}