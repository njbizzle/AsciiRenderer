#include <iostream>

#include <Eigen/Dense>
#include <cmath>
#include <math.h>
#include <algorithm>
#include <cstdlib>
#include <functional>

// x forward, y left, z up

float WIDTH = 30;
float HEIGHT = 30;

float VIEW_W = 6;
float VIEW_H = 6;

float big_r = 2;
float little_r = 1;

Eigen::Vector3f look_from = {0,0,5};
Eigen::Vector3f look_at = {0,0,0};
Eigen::Vector3f camera_relative_up = {0,1,0};


Eigen::Vector3f rot_axis = {1,1,1}; // will be normalized
float steps_per_rot = 15; // will be normalized


int max_iter = 10;
float tolerance = 1e-4;

float one_over_tolerance = 1 / tolerance;

Eigen::Vector3f fdu, fdv, fdt;

Eigen::Matrix3f jacobian;
Eigen::Vector3f fx;

Eigen::Vector3f e_du = Eigen::Vector3f{tolerance, 0, 0};
Eigen::Vector3f e_dv = Eigen::Vector3f{0, tolerance, 0};
Eigen::Vector3f e_dt = Eigen::Vector3f{0, 0, tolerance};

Eigen::Matrix4f quat_to_matrix(Eigen::Vector4f q) {
	// derived from the relations i^2 = j^2 = k^2 = ijk = -1
	float a = q[0];
	float b = q[1];
	float c = q[2];
	float d = q[3];
	return Eigen::Matrix4f{
        {  a, -b, -c, -d},
        {  b,  a, -d,  c},
        {  c,  d,  a, -b},
        {  d, -c,  b,  a}
	};
}

Eigen::Vector4f quat_mult(Eigen::Vector4f q1, Eigen::Vector4f q2) {
	return quat_to_matrix(q1) * q2;
}

void set_up_quats(float theta, Eigen::Vector3f& axis, Eigen::Vector4f& q, Eigen::Vector4f& q_conj) {
	Eigen::Vector3f v = axis.normalized() * sin(theta * 0.5);
	q = {cos(theta * 0.5), v[0], v[1], v[2]};
	q_conj = {cos(theta * 0.5), -v[0], -v[1], -v[2]};
}

Eigen::Matrix4f quat_rot_homog(float theta, Eigen::Vector3f axis) {
	Eigen::Vector4f q, q_conj;
	set_up_quats(theta, axis, q, q_conj);
	float q0 = q[0];
	float q1 = q[1];
	float q2 = q[2];
	float q3 = q[3];
	float r00, r01, r02, r10, r11, r12, r20, r21, r22;
	// https://automaticaddison.com/how-to-convert-a-quaternion-to-a-rotation-matrix/
	// this article had some code to copy paste so i don't have to make any typos
	// alterations were made to turn it from python to cpp and to make it into a homogenous coordinate system
	r00 = 2 * (q0 * q0 + q1 * q1) - 1;
    r01 = 2 * (q1 * q2 - q0 * q3);
    r02 = 2 * (q1 * q3 + q0 * q2);

    r10 = 2 * (q1 * q2 + q0 * q3);
    r11 = 2 * (q0 * q0 + q2 * q2) - 1;
    r12 = 2 * (q2 * q3 - q0 * q1);

    r20 = 2 * (q1 * q3 - q0 * q2);
    r21 = 2 * (q2 * q3 + q0 * q1);
    r22 = 2 * (q0 * q0 + q3 * q3) - 1;

	return Eigen::Matrix4f {
		{r00, r01, r02, 0},
		{r10, r11, r12, 0},
		{r20, r21, r22, 0},
		{0,0,0,1},
	};
}

void rotate_vec_quat(Eigen::Vector3f& vec, float theta, Eigen::Vector3f& axis) {
	float len = vec.norm();
	vec.normalize();
	Eigen::Vector4f out, q, q_conj;
	set_up_quats(theta, axis, q, q_conj);
	out = len * quat_mult(quat_mult(q, {0, vec[0], vec[1], vec[2]}), q_conj);
	/* can you tell there was a lot of debugging
	std::cout << "q   " << q << "\n";
	std::cout << "q-1 " << q_conj << "\n";
	std::cout << "vec " << vec << "\n";
	std::cout << "a " << quat_mult(q, {0, vec[0], vec[1], vec[2]}) << "\n";
	std::cout << "out " << out << "\n\n\n";
	*/
	vec = {out[1], out[2], out[3]};
}

float rand_() {
	return 2 * (((float) rand()) / (RAND_MAX)) - 1;
}

Eigen::Vector3f estim_zeros_recur(std::function<Eigen::Vector3f(Eigen::Vector3f)>  f, Eigen::Vector3f x, int iter){
	// std::cout << iter << " : " << x[0] << " "  << x[1] << " "  << x[2] << "\n";
	if (iter > max_iter) {
		return {NAN,NAN,NAN};
	}
	fx = f(x);

	// std::cout << iter << " - " << fx.norm() << " - " << x << "\n" ;
	if (fx.norm() < tolerance) {
		// std::cout << "\nsuccess B)\n\n";
		return x;
	}

	jacobian.col(0) = (f(x + e_du) - fx) / tolerance;
	jacobian.col(1) = (f(x + e_dv) - fx) / tolerance;
	jacobian.col(2) = (f(x + e_dt) - fx) / tolerance;

	// std::cout << jacobian << "\n\n";

	return estim_zeros_recur(f, x - (jacobian.inverse() * fx), iter + 1);
}

Eigen::Vector3f estim_zeros(std::function<Eigen::Vector3f(Eigen::Vector3f)> f, Eigen::Vector3f init_guess){
	return estim_zeros_recur(f, init_guess, 0);
}

Eigen::Vector3f torus(float u, float v, float r1, float r2, Eigen::Matrix4f transform) {
	Eigen::Vector4f out = Eigen::Vector4f {
		(r2 * cos(2 * M_PI * u) + r1) * (cos(2 * M_PI * v)),
		(r2 * cos(2 * M_PI * u) + r1) * (sin(2 * M_PI * v)),
		(r2) * (sin(2 * M_PI * u)),
		1
	};
	out = transform * out;
	return Eigen::Vector3f {
		out[0] / out[3],
		out[1] / out[3],
		out[2] / out[3],
	};
}

Eigen::Vector3f ray(float t, Eigen::Vector3f center, Eigen::Vector3f dir) {
	return center + t*dir;
}

Eigen::Vector3f estim_torus(Eigen::Vector3f center, Eigen::Vector3f dir, float r1, float r2, Eigen::Matrix4f transform){
	Eigen::Vector3f rv1 = {rand_(), rand_(), rand_()};

	Eigen::Vector3f estim = estim_zeros(
		[=](Eigen::Vector3f v) -> Eigen::Vector3f {
			return torus(v[0], v[1], r1, r2, transform) - ray(v[2], center, dir);
		},
		rv1
	);
	return estim;
}


float get_depth(Eigen::Vector3f center, Eigen::Vector3f dir, float r1, float r2, Eigen::Matrix4f transform){
	return estim_torus(center, dir.normalized(), r1, r2, transform)[2];
}

Eigen::Vector3f get_intersection(Eigen::Vector3f center, Eigen::Vector3f dir, float r1, float r2, Eigen::Matrix4f transform){
	Eigen::Vector3f estim = estim_torus(center, dir, r1, r2, transform);
	return torus(estim[0], estim[1], r1, r2, transform);
}

Eigen::Vector3f c_w, c_u, c_v, c_du, c_dv, camera_00;

void calc_camera(Eigen::Vector3f from, Eigen::Vector3f at) {
	// i dont care about graphics standards, positive is to the right
	c_w = (at - from).normalized(); // forward
	c_u = - camera_relative_up.cross(c_w).normalized(); // right
	c_v = - c_w.cross(c_u); // up

	camera_00 = from
		- c_u * (VIEW_W / 2)
		- c_v * (VIEW_H / 2);

	c_du = c_u  * (VIEW_W / (WIDTH-1));
	c_dv = c_v  * (VIEW_H / (HEIGHT-1));
}


float get_luminance(int i, int j, float t) {
	Eigen::Vector3f pix_center = camera_00
		+ c_du * i
		+ c_dv * j;

	float min_d = 9999;
	float d;

	Eigen::Matrix4f transform = Eigen::Matrix4f::Identity();
	Eigen::Vector3f offset = {0,0,0};
	transform(0,3) = offset[0];
	transform(1,3) = offset[1];
	transform(2,3) = offset[2];

	float ang_rad = fmod(
		(t * 2 * M_PI) / steps_per_rot,
		2 * M_PI
	);
	transform *= quat_rot_homog(ang_rad, {1,1,1});
	// std::cout << transform << "\n\n";

	for (int iter = 0; iter < 5; iter++){
		d = get_depth(pix_center, c_w, big_r, little_r, transform);
		min_d = (d < min_d) && (!isnan(d)) ? d : min_d;
	}

    return min_d;
};

float t = 0;
float dt = 1;

void onLoop() {
	std::string output = "";

	std::vector<float> luminances = std::vector<float>{};

	float min = 999;
	float max = -999;

	t += dt;

	// calc_camera({7.07 * cos(t), 7.07 * sin(t), 5}, look_at);

	calc_camera(look_from + Eigen::Vector3f{1e-5,1e-5,1e-5}, look_at); // exactly 0 gives some strange things
    for (int j = 0; j < HEIGHT; j++) {
        for (int i = 0; i < WIDTH; i++) {

			float l = get_luminance(i,j,t);
			luminances.push_back(l);
			if (isnan(l) || l > 99) {
				continue;
			}
			min = (l < min) ? l : min;
			max = (l > max) ? l : max;
        }
    }

	// std::clog << min << " " << max;

	std::string color_grad = "#0*^. ";
	int i = 0;
	for (float l : luminances) {
		int index = floor(color_grad.length() * ((l - min) / (max - min)));
		index = (index > 0) ? index : 0;
		index = (index < color_grad.length()) ? index : color_grad.length() - 1;

		output += color_grad[index];
		output += ' ';
		if (++i == WIDTH) {
			i = 0;
			output += '\n';
		}
	}

    system("clear");
	std::clog << output;
};

int main() {
    while (true) {
        onLoop();
		// std::cin.ignore();
    }
    return 0;
};
