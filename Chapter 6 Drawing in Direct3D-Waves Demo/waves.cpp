#include"waves.h"
#include<algorithm>
#include<vector>
#include<cassert>

Waves::Waves() = default;

Waves::~Waves()
{
	delete[] prev_solution_;
	delete[] curr_solution_;
}



void Waves::Init(UINT m, UINT n, float dx, float dt, float speed, float damping)
{
	num_rows_ = m;
	num_cols_ = n;

	vertex_cnt_ = m*n;
	tri_cnt_ = (m - 1)*(n - 1) * 2;

	time_step_ = dt;
	spatial_step_ = dx;

	float d = damping*dt + 2.0f;
	float e = (speed*speed)*(dt*dt) / (dx*dx);
	k1_ = (damping*dt - 2.0f) / d;
	k2_ = (4.0f - 8.0f*e) / d;
	k3_ = (2.0f*e) / d;

	// In case Init() called again.
	delete[] prev_solution_;
	delete[] curr_solution_;

	prev_solution_ = new DirectX::XMFLOAT3[m*n];
	curr_solution_ = new DirectX::XMFLOAT3[m*n];

	// Generate grid vertices in system memory.

	float halfWidth = (n - 1)*dx*0.5f;
	float halfDepth = (m - 1)*dx*0.5f;
	for (UINT i = 0; i < m; ++i)
	{
		float z = halfDepth - i*dx;
		for (UINT j = 0; j < n; ++j)
		{
			float x = -halfWidth + j*dx;

			prev_solution_[i*n + j] = DirectX::XMFLOAT3(x, 0.0f, z);
			curr_solution_[i*n + j] = DirectX::XMFLOAT3(x, 0.0f, z);
		}
	}
}

void Waves::Update(float dt)
{
	static float t = 0;

	// Accumulate time.
	t += dt;

	// Only update the simulation at the specified time step.
	if (t >= time_step_)
	{
		// Only update interior points; we use zero boundary conditions.
		for (DWORD i = 1; i < num_rows_ - 1; ++i)
		{
			for (DWORD j = 1; j < num_cols_ - 1; ++j)
			{
				// After this update we will be discarding the old previous
				// buffer, so overwrite that buffer with the new update.
				// Note how we can do this inplace (read/write to same element) 
				// because we won't need prev_ij again and the assignment happens last.

				// Note j indexes x and i indexes z: h(x_j, z_i, t_k)
				// Moreover, our +z axis goes "down"; this is just to 
				// keep consistent with our row indices going down.

				prev_solution_[i*num_cols_ + j].y =
					k1_*prev_solution_[i*num_cols_ + j].y +
					k2_*curr_solution_[i*num_cols_ + j].y +
					k3_*(curr_solution_[(i + 1)*num_cols_ + j].y +
						curr_solution_[(i - 1)*num_cols_ + j].y +
						curr_solution_[i*num_cols_ + j + 1].y +
						curr_solution_[i*num_cols_ + j - 1].y);
			}
		}

		// We just overwrote the previous buffer with the new data, so
		// this data needs to become the current solution and the old
		// current solution becomes the new previous solution.
		std::swap(prev_solution_, curr_solution_);

		t = 0.0f; // reset time
	}
}

void Waves::Disturb(UINT i, UINT j, float magnitude)
{
	// Don't disturb boundaries.
	assert(i > 1 && i < num_rows_ - 2);
	assert(j > 1 && j < num_cols_ - 2);

	float halfMag = 0.5f*magnitude;

	// Disturb the ijth vertex height and its neighbors.
	curr_solution_[i*num_cols_ + j].y += magnitude;
	curr_solution_[i*num_cols_ + j + 1].y += halfMag;
	curr_solution_[i*num_cols_ + j - 1].y += halfMag;
	curr_solution_[(i + 1)*num_cols_ + j].y += halfMag;
	curr_solution_[(i - 1)*num_cols_ + j].y += halfMag;
}

