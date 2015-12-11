// Angle estimator based on gyro velocities
// Author: Max Schwarz <max.schwarz@uni-bonn.de>

#ifndef ANGLEESTIMATOR_H
#define ANGLEESTIMATOR_H
#include <string>

class minIni;
class AngleEstimator
{
	public:
		AngleEstimator();
		virtual ~AngleEstimator();

		void predict(double gyro_dpitch, double gyro_droll, double gyro_dyaw);
		void update(double acc_x, double acc_y, double acc_z);

		void setAccGain(double acc_gain);
		void setAccSmoothDecay(double acc_smooth_decay);

		void LoadINISettings(minIni* ini, const std::string& section = "AngleEstimator");
		void SaveINISettings(minIni* ini, const std::string& section = "AngleEstimator");

		inline double accGain() const
		{ return m_acc_gain; }

		inline double accSmoothDecay() const
		{ return m_acc_smooth_decay; }

		inline double roll() const
		{ return m_roll; }

		inline double pitch() const
		{ return m_pitch; }

		inline double yaw() const
		{ return m_yaw; }
	private:
		double m_acc_gain;
		double m_acc_smooth_decay;

		double m_acc_x;
		double m_acc_y;
		double m_acc_z;

		double m_roll;
		double m_pitch;
		double m_yaw;
};

#endif