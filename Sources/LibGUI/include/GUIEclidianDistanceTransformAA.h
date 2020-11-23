#pragma once

class GUIEclidianDistanceTransformAA
{
	protected:
		size_t			m_MemorySize; // = szWidth * szHeight
		size_t			m_szWidth;
		size_t			m_szHeight;

		double*			m_pImage;
		RT::Vector2d*	m_pGradiantPts;

		void ComputeGradient(RT::Vector2d* pGradiant) const;
		inline void DistanceComputeUpdate(const size_t szIndex, const size_t szDistanceIndex, RT::Vector2us* pDistancePts, const RT::Vector2us& direction, double& dOldDistance, double* pDistancePicture, bool& bChanged) const;
		inline void DistanceCompute(const size_t szIndex, const size_t szDistanceIndex, RT::Vector2us* pDistancePts, const RT::Vector2us& direction, const double dOldDistance, double* pDistancePicture, bool& bChanged) const;

		double edgedf(const RT::Vector2d& gradiant, const double a) const;
		double distaa3(const double* pImage, const RT::Vector2d* pGradiantPts, const size_t closest, const RT::Vector2us& newDistanceDirection) const;
		void edtaa3(RT::Vector2us* pDistancePts, double* dist) const;

	public:
		GUIEclidianDistanceTransformAA(void):
		m_MemorySize(0), m_szWidth(0), m_szHeight(0), m_pImage(NULL), m_pGradiantPts(NULL)
		{}

		virtual ~GUIEclidianDistanceTransformAA(void)
		{}

		void distanceMap(double* data, const size_t width, const size_t height);
};

