#pragma once
#include "Math.h"
#include "vector"

namespace dae
{
	struct Vertex
	{
		Vector3 position{};
		ColorRGB color{colors::White};
		//Vector2 uv{}; //W3
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
		
		void Grow(Vector2 position)
		{
			min.x = std::min(min.x, position.x);
			min.y = std::min(min.y, position.y);

			max.x = std::max(max.x, position.x);
			max.y = std::max(max.y, position.y);
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