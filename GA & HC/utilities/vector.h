#pragma once
struct vector2 {
	float x = 0.f, y = 0.f;

	constexpr vector2(const float x = 0.f, const float y = 0.f) : x(x), y(y) {}

	bool operator == (const vector2& vec) const {
		return (this->x == vec.x && this->y == vec.y);
	}

	vector2 operator + (const vector2& vec) const {
		return vector2(this->x + vec.x, this->y + vec.y);
	}

	vector2 operator - (const vector2& vec) const {
		return vector2(this->x - vec.x, this->y - vec.y);
	}

	vector2 operator / (const vector2& vec) const {
		return vector2(this->x / vec.x, this->y / vec.y);
	}

	vector2 operator * (const float f) const {
		return vector2(this->x * f, this->y * f);
	}
};
