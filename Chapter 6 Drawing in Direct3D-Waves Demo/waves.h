#ifndef WAVES_H
#define WAVES_H

#include<Windows.h>
#include"d3dutility.h" 

class Waves
{
public:
	Waves();
	~Waves();

	inline UINT RowCount()const { return num_rows_; }
	inline UINT ColumnCount()const { return num_cols_; }
	inline UINT VertexCount()const { return vertex_cnt_; }
	inline UINT TriangleCount()const { return tri_cnt_; }

	// Returns the solution at the ith grid point.
	inline const DirectX::XMFLOAT3 &operator[](int i)const { return curr_solution_[i]; }

	void Init(UINT m, UINT n, float dx, float dt, float speed, float damping);
	void Update(float dt);
	void Disturb(UINT i, UINT j, float magnitude);

private:
	UINT num_rows_ = 0;
	UINT num_cols_ = 0;

	UINT vertex_cnt_ = 0;
	UINT tri_cnt_ = 0;

	// Simulation constants we can precompute.
	float k1_ = 0.0f;
	float k2_ = 0.0f;
	float k3_ = 0.0f;

	float time_step_ = 0.0f;
	float spatial_step_ = 0.0f;

	DirectX::XMFLOAT3* prev_solution_ = nullptr;
	DirectX::XMFLOAT3* curr_solution_ = nullptr;
};

#endif // WAVES_H
