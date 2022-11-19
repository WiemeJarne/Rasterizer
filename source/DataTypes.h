#pragma once
#include "Math.h"
#include "vector"

namespace dae
{
	struct Vertex
	{
		Vector3 position{};
		ColorRGB color{colors::White};
		Vector2 uv{}; //W3
		//Vector3 normal{}; //W4
		//Vector3 tangent{}; //W4
		//Vector3 viewDirection{}; //W4
	};

	struct Vertex_Out
	{
		Vector4 position{};
		ColorRGB color{ colors::White };
		//Vector2 uv{};
		//Vector3 normal{};
		//Vector3 tangent{};
		//Vector3 viewDirection{};
	};

	enum class PrimitiveTopology
	{
		TriangeList,
		TriangleStrip
	};

	struct Mesh
	{
		std::vector<Vertex> vertices{};
		std::vector<uint32_t> indices{};
		PrimitiveTopology primitiveTopology{ PrimitiveTopology::TriangleStrip };

		std::vector<Vertex_Out> vertices_out{};
		Matrix worldMatrix{};
	};

	struct TriangleBoundingBox
	{
		Vector2 min;
		Vector2 max;
		
		int screenWith;
		int screenHeight;
		
		void Grow(Vector2 position)
		{
			min.x = std::min(min.x, position.x);
			min.y = std::min(min.y, position.y);

			max.x = std::max(max.x, position.x);
			max.y = std::max(max.y, position.y);

			if (min.x < 0) min.x = 0;
			if (min.y < 0) min.y = 0;
			if (max.x > screenWith) max.x = screenWith;
			if (max.y > screenHeight) max.y = screenHeight;

			if (min.x > screenWith) min.x = screenWith;
			if (min.y > screenHeight) min.y = screenHeight;
			if (max.x < 0) max.x = 0;
			if (max.y < 0) max.y = 0;
		}

		bool IsPointInBoundingBox(Vector2 position)
		{
			if (	position.x > min.x && position.x < max.x
				 && position.y > min.y && position.y < max.y )
			{
				return true;
			}
			return false;
		}
	};
}