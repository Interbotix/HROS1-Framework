// Angle estimator based on gyro velocities
// Author: Max Schwarz <max.schwarz@uni-bonn.de>

#include "AngleEstimator.h"
#include <minIni.h>

#include <math.h>

AngleEstimator::AngleEstimator()
 : m_acc_gain(0.01)
 , m_acc_smooth_decay(0.01)
{
}

AngleEstimator::~AngleEstimator()
{
}

void AngleEstimator::predict(double gyro_dpitch, double gyro_droll, double gyro_dyaw)
{
	m_pitch += gyro_dpitch;
	m_roll  += gyro_droll;
}

void AngleEstimator::update(double x, double y, double z)
{
	m_acc_x = (1.0 - m_acc_smooth_decay) * m_acc_x + m_acc_smooth_decay * x;
	m_acc_y = (1.0 - m_acc_smooth_decay) * m_acc_y + m_acc_smooth_decay * y;
	m_acc_z = (1.0 - m_acc_smooth_decay) * m_acc_z + m_acc_smooth_decay * z;

	double acc_roll = atan2(m_acc_x, m_acc_z);
	double acc_pitch = atan2(-m_acc_y, sqrt(m_acc_x*m_acc_x+m_acc_z*m_acc_z));

	m_pitch = (1.0 - m_acc_gain) * m_pitch + m_acc_gain * acc_pitch;
	m_roll = (1.0 - m_acc_gain) * m_roll   + m_acc_gain * acc_roll;
}

void AngleEstimator::LoadINISettings(minIni* ini, const std::string& section)
{
	double value;
	if((value = ini->getd(section, "acc_smooth_decay", -1)) >= 0)
		m_acc_smooth_decay = value;
	if((value = ini->getd(section, "acc_gain", -1)) >= 0)
		m_acc_gain = value;
}

void AngleEstimator::SaveINISettings(minIni* ini, const std::string& section)
{
	ini->put(section, "acc_smooth_decay", m_acc_smooth_decay);
	ini->put(section, "acc_gain", m_acc_gain);
}
