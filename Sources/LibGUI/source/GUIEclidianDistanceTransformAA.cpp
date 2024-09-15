#include "Dependencies.h"
#include "LibGUI/include/GUIEclidianDistanceTransformAA.h"

/*
 * Compute the local gradient at edge pixels using convolution filters.
 * The gradient is computed only at edge pixels. At other places in the
 * image, it is never used, and it's mostly zero anyway.
 */
void GUIEclidianDistanceTransformAA::ComputeGradient(RT::Vector2d* pGradiant) const
{
	MouCa::assertion(m_pImage!=NULL && m_MemorySize > 1);	//DEV Issue: Initialization missing or failed
	MouCa::assertion(pGradiant!=NULL);

	for(size_t uiY = 1; uiY < m_szHeight-1; uiY++)
	{ // Avoid edges where the kernels would spill over
		const size_t szLine = uiY*m_szWidth;
		for(size_t uiX = 1; uiX < m_szWidth-1; uiX++)
		{
			const size_t szLinearID = szLine + uiX;
			if((0.0 < m_pImage[szLinearID]) && (m_pImage[szLinearID]<1.0)) // Compute gradient for edge pixels only
			{ 
				const double dGlobalPart = -m_pImage[szLinearID-m_szWidth-1] - m_pImage[szLinearID+m_szWidth-1] + m_pImage[szLinearID-m_szWidth+1] + m_pImage[szLinearID+m_szWidth+1];
				RT::Vector2d gradient = RT::Vector2d(	dGlobalPart - RT::g_SQRT_2*m_pImage[szLinearID-1]		 + RT::g_SQRT_2*m_pImage[szLinearID+1],
														dGlobalPart - RT::g_SQRT_2*m_pImage[szLinearID-m_szWidth]  + RT::g_SQRT_2*m_pImage[szLinearID+m_szWidth]);

				const double dGradiantLength = glm::dot(gradient, gradient);
				if(dGradiantLength > 0.0) // Avoid division by zero
				{
					gradient *= 1.0/sqrt(dGradiantLength);
				}
				pGradiant[szLinearID] = gradient;
			}
		}
	}
	// TODO: Compute reasonable values for gx, gy also around the image edges.
	// (These are zero now, which reduces the accuracy for a 1-pixel wide region
	// around the image edge.) 2x2 kernels would be suitable for this.
}

/*
 * A somewhat tricky function to approximate the distance to an edge in a
 * certain pixel, with consideration to either the local gradient (gx,gy)
 * or the direction to the pixel (dx,dy) and the pixel greyscale value a.
 * The latter alternative, using (dx,dy), is the metric used by edtaa2().
 * Using a local estimate of the edge gradient (gx,gy) yields much better
 * accuracy at and near edges, and reduces the error even at distant pixels
 * provided that the gradient direction is accurately estimated.
 */
double GUIEclidianDistanceTransformAA::edgedf(const RT::Vector2d& gradiant, const double a) const
{
	double df;

	if((gradiant.x == 0.0) || (gradiant.y == 0.0))
	{ // Either A) gu or gv are zero, or B) both
		df = 0.5-a;  // Linear approximation is A) correct or B) a fair guess
	}
	else
	{
		const double glength = 1.0/sqrt(glm::dot(gradiant, gradiant));
		MouCa::assertion(glength>0.0);
		
		/* Everything is symmetric wrt sign and transposition,
		 * so move to first octant (gx>=0, gy>=0, gx>=gy) to
		 * avoid handling all possible edge directions.
		 */
		RT::Vector2d normGradiant = glm::abs(gradiant*glength);
		if(normGradiant.x<normGradiant.y)
		{
			std::swap(normGradiant.x, normGradiant.y);
		}
		const double a1 = 0.5*normGradiant.y/normGradiant.x;
		if (a < a1)
		{ // 0 <= a < a1
			df = 0.5*(normGradiant.x + normGradiant.y) - sqrt(2.0*normGradiant.x*normGradiant.y*a);
		}
		else if (a < (1.0-a1))
		{ // a1 <= a <= 1-a1
			df = (0.5-a)*normGradiant.x;
		}
		else
		{ // 1-a1 < a <= 1
			df = -0.5*(normGradiant.x + normGradiant.y) + sqrt(2.0*normGradiant.x*normGradiant.y*(1.0-a));
		}
	}    
	return df;
}

double GUIEclidianDistanceTransformAA::distaa3(const double* pImage, const RT::Vector2d* pGradiantPts, const size_t closest, const RT::Vector2us& newDistanceDirection) const
{
	//const size_t closest = c-xc-yc*w; // Index to the edge pixel pointed to from c
	double a = std::clamp(m_pImage[closest], 0.0, 1.0);    // Grayscale value at the edge pixel
	if(a == 0.0)
		return 1000000.0; // Not an object pixel, return "very far" ("don't know yet")

	const RT::Vector2d& gradiant = pGradiantPts[closest]; // gradient component at the edge pixel
	
	double df;
	if(newDistanceDirection.x==0 && newDistanceDirection.y==0)
	{
		// Use local gradient only at edges, Estimate based on local gradient only
		df = edgedf(gradiant, a);
	}
	else
	{
		const RT::Vector2d derivate((double)newDistanceDirection.x, (double)newDistanceDirection.y);
		const double di = sqrt(glm::dot(derivate, derivate)); // Length of integer vector, like a traditional EDT

		// Estimate gradient based on direction to edge (accurate for large di)
		df = di + edgedf(derivate, a); // Same metric as edtaa2, except at edges (where di=0)
	}
	return df;
}

// Shorthand macro: add ubiquitous parameters dist, gx, gy, img and w and call distaa3()
#define DISTAA(c,xc,yc,xi,yi) (distaa3(pImage, pGradiantPts, uiWidth, c, xc, yc, xi, yi))

void GUIEclidianDistanceTransformAA::DistanceComputeUpdate(const size_t szIndex, const size_t szDistanceIndex, RT::Vector2us* pDistancePts, const RT::Vector2us& direction, double& dOldDistance, double* pDistancePicture, bool& bChanged) const
{
	const double epsilon = 1e-3;
	const RT::Vector2us& cdist = pDistancePts[szIndex];
	RT::Vector2us newDistanceDirection = cdist + direction;

	MouCa::assertion((size_t)cdist.x-(size_t)cdist.y*m_szWidth <= szIndex);
	const size_t closest = szIndex-(size_t)cdist.x-(size_t)cdist.y*m_szWidth;
	const double newDistance = distaa3(m_pImage, m_pGradiantPts, closest, newDistanceDirection);
	if(newDistance < dOldDistance-epsilon)
	{
		pDistancePts[szDistanceIndex]		= newDistanceDirection;
		pDistancePicture[szDistanceIndex]	= newDistance;
		dOldDistance=newDistance;
		bChanged = true;
	}
}

void GUIEclidianDistanceTransformAA::DistanceCompute(const size_t szIndex, const size_t szDistanceIndex, RT::Vector2us* pDistancePts, const RT::Vector2us& direction, const double dOldDistance, double* pDistancePicture, bool& bChanged) const
{
	const double epsilon = 1e-3;
	const RT::Vector2us& cdist = pDistancePts[szIndex];
	RT::Vector2us newDistanceDirection = cdist + direction;

	MouCa::assertion((size_t)cdist.x-(size_t)cdist.y*m_szWidth <= szIndex);
	const size_t closest = szIndex-(size_t)cdist.x-(size_t)cdist.y*m_szWidth;
	const double newDistance = distaa3(m_pImage, m_pGradiantPts, closest, newDistanceDirection);
	if(newDistance < dOldDistance-epsilon)
	{
		pDistancePts[szDistanceIndex]		= newDistanceDirection;
		pDistancePicture[szDistanceIndex]	= newDistance;
		bChanged = true;
	}
}

void GUIEclidianDistanceTransformAA::edtaa3(RT::Vector2us* pDistancePts, double* pDistance) const
{
//	int x, c;
	__int64 offset_u, offset_ur, offset_r, offset_rd,
	offset_d, offset_dl, offset_l, offset_lu;
	double olddist;//, newdist;
//	int cdistx, cdisty, newdistx, newdisty;
//	const double epsilon = 1e-3;

	// Initialize index offsets for the current image width
	offset_u = -(__int64)m_szWidth;
	offset_ur = -(__int64)m_szWidth+1;
	offset_r = 1;
	offset_rd = m_szWidth+1;
	offset_d = m_szWidth;
	offset_dl = m_szWidth-1;
	offset_l = -1;
	offset_lu = -(__int64)m_szWidth-1;

	// Initialize the distance images
	for(size_t szPixel=0; szPixel < m_MemorySize; szPixel++)
	{
		pDistancePts[szPixel] = RT::Vector2us(0, 0); // At first, all pixels point to themselves as the closest known.
		if(m_pImage[szPixel] <= 0.0)
		{
			pDistance[szPixel] = 1000000.0; // Big value, means "not set yet"
		}
		else if(m_pImage[szPixel] < 1.0)
		{
			pDistance[szPixel] = edgedf(m_pGradiantPts[szPixel], m_pImage[szPixel]); // Gradient-assisted estimate
		}
		else
		{
			pDistance[szPixel] = 0.0; // Inside the object
		}
	}

	/* Perform the transformation */
	bool bChanged;
	do
	{
		bChanged = false;

		size_t i;

		/* Scan rows, except first row */
		for(size_t szY=1; szY<m_szHeight; szY++)
		{
			/* move index to leftmost pixel of current row */
			i = szY*m_szWidth;

			/* scan right, propagate distances from above & left */

			/* Leftmost pixel is special, has no left neighbors */
			olddist = pDistance[i];
			if(olddist > 0) // If non-zero distance or not set yet
			{
				DistanceComputeUpdate(i + offset_u, i, pDistancePts, RT::Vector2us(0, 1), olddist, pDistance, bChanged);
/*				c = i + offset_u; // Index of candidate for testing
				cdistx = pDistancePts[c];
				cdisty = disty[c];
				newdistx = cdistx;
				newdisty = cdisty+1;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					olddist=newdist;
					bChanged = 1;
				}
*/
				DistanceCompute(i + offset_ur, i, pDistancePts, RT::Vector2us(-1, 1), olddist, pDistance, bChanged);
/*				c = i+offset_ur;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx-1;
				newdisty = cdisty+1;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					bChanged = 1;
				}
*/
			}
			i++;

			/* Middle pixels have all neighbors */
			for(size_t x=1; x<m_szWidth-1; x++, i++)
			{
				olddist = pDistance[i];
				if(olddist <= 0) continue; // No need to update further

				DistanceComputeUpdate(i + offset_l, i, pDistancePts, RT::Vector2us(1, 0), olddist, pDistance, bChanged);
/*				c = i+offset_l;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx+1;
				newdisty = cdisty;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					olddist=newdist;
					bChanged = 1;
				}
*/
				DistanceComputeUpdate(i + offset_lu, i, pDistancePts, RT::Vector2us(1, 1), olddist, pDistance, bChanged);
/*				c = i+offset_lu;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx+1;
				newdisty = cdisty+1;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					olddist=newdist;
					bChanged = 1;
				}
*/
				DistanceComputeUpdate(i + offset_u, i, pDistancePts, RT::Vector2us(0, 1), olddist, pDistance, bChanged);
/*				c = i+offset_u;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx;
				newdisty = cdisty+1;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					olddist=newdist;
					bChanged = 1;
				}
*/
				DistanceCompute(i + offset_ur, i, pDistancePts, RT::Vector2us(-1, 1), olddist, pDistance, bChanged);
/*
				c = i+offset_ur;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx-1;
				newdisty = cdisty+1;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					bChanged = 1;
				}
*/
			}

			/* Rightmost pixel of row is special, has no right neighbors */
			olddist = pDistance[i];
			if(olddist > 0) // If not already zero distance
			{
				DistanceComputeUpdate(i + offset_l, i, pDistancePts, RT::Vector2us(1, 0), olddist, pDistance, bChanged);
/*
				c = i+offset_l;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx+1;
				newdisty = cdisty;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					olddist=newdist;
					bChanged = 1;
				}
*/
				DistanceComputeUpdate(i + offset_lu, i, pDistancePts, RT::Vector2us(1, 1), olddist, pDistance, bChanged);
/*
				c = i+offset_lu;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx+1;
				newdisty = cdisty+1;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					olddist=newdist;
					bChanged = 1;
				}
*/
				DistanceCompute(i + offset_u, i, pDistancePts, RT::Vector2us(0, 1), olddist, pDistance, bChanged);
/*
				c = i+offset_u;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx;
				newdisty = cdisty+1;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					bChanged = 1;
				}
*/
			}

			/* Move index to second rightmost pixel of current row. */
			/* Rightmost pixel is skipped, it has no right neighbor. */
			i = szY*m_szWidth + m_szWidth-2;

			/* scan left, propagate distance from right */
			for(__int64 x=(__int64)m_szWidth-2; x>=0; x--, i--)
			{
				olddist = pDistance[i];
				if(olddist <= 0) continue; // Already zero distance

				DistanceCompute(i + offset_r, i, pDistancePts, RT::Vector2us(-1, 0), olddist, pDistance, bChanged);
/*				c = i+offset_r;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx-1;
				newdisty = cdisty;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					bChanged = 1;
				}
*/
			}
		}
	  
		/* Scan rows in reverse order, except last row */
		for(__int64 szY=(__int64)m_szHeight-2; szY>=0; szY--)
		{
			/* move index to rightmost pixel of current row */
			i = szY*m_szWidth + m_szWidth-1;

			/* Scan left, propagate distances from below & right */

			/* Rightmost pixel is special, has no right neighbors */
			olddist = pDistance[i];
			if(olddist > 0) // If not already zero distance
			{
				DistanceComputeUpdate(i + offset_d, i, pDistancePts, RT::Vector2us(0, -1), olddist, pDistance, bChanged);
				/*
				c = i+offset_d;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx;
				newdisty = cdisty-1;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					olddist=newdist;
					bChanged = 1;
				}
				*/

				DistanceCompute(i + offset_dl, i, pDistancePts, RT::Vector2us(1, -1), olddist, pDistance, bChanged);
				/*
				c = i+offset_dl;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx+1;
				newdisty = cdisty-1;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					bChanged = 1;
				}
				*/
			}
			i--;

			/* Middle pixels have all neighbors */
			for(__int64 x=(__int64)m_szWidth-2; x>0; x--, i--)
			{
				olddist = pDistance[i];
				if(olddist <= 0) continue; // Already zero distance

				DistanceComputeUpdate(i + offset_r, i, pDistancePts, RT::Vector2us(-1, 0), olddist, pDistance, bChanged);
				/*
				c = i+offset_r;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx-1;
				newdisty = cdisty;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					olddist=newdist;
					bChanged = 1;
				}
				*/

				DistanceComputeUpdate(i + offset_rd, i, pDistancePts, RT::Vector2us(-1, -1), olddist, pDistance, bChanged);
				/*
				c = i+offset_rd;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx-1;
				newdisty = cdisty-1;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					olddist=newdist;
					bChanged = 1;
				}
				*/

				DistanceComputeUpdate(i + offset_d, i, pDistancePts, RT::Vector2us(0, -1), olddist, pDistance, bChanged);
				/*
				c = i+offset_d;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx;
				newdisty = cdisty-1;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					olddist=newdist;
					bChanged = 1;
				}
				*/
				DistanceCompute(i + offset_dl, i, pDistancePts, RT::Vector2us(1, -1), olddist, pDistance, bChanged);
				/*
				c = i+offset_dl;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx+1;
				newdisty = cdisty-1;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					bChanged = 1;
				}
				*/
			}
			/* Leftmost pixel is special, has no left neighbors */
			olddist = pDistance[i];
			if(olddist > 0) // If not already zero distance
			{
				DistanceComputeUpdate(i + offset_r, i, pDistancePts, RT::Vector2us(-1, 0), olddist, pDistance, bChanged);
				/*
				c = i+offset_r;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx-1;
				newdisty = cdisty;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					olddist=newdist;
					bChanged = 1;
				}
				*/

				DistanceComputeUpdate(i + offset_rd, i, pDistancePts, RT::Vector2us(-1, -1), olddist, pDistance, bChanged);
				/*
				c = i+offset_rd;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx-1;
				newdisty = cdisty-1;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					olddist=newdist;
					bChanged = 1;
				}
				*/
				DistanceCompute(i + offset_d, i, pDistancePts, RT::Vector2us(0, -1), olddist, pDistance, bChanged);
				/*
				c = i+offset_d;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx;
				newdisty = cdisty-1;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					bChanged = 1;
				}
				*/
			}

			/* Move index to second leftmost pixel of current row. */
			/* Leftmost pixel is skipped, it has no left neighbor. */
			i = szY*m_szWidth + 1;
			for(size_t x=1; x<m_szWidth; x++, i++)
			{
				/* scan right, propagate distance from left */
				olddist = pDistance[i];
				if(olddist <= 0) continue; // Already zero distance

				DistanceCompute(i + offset_l, i, pDistancePts, RT::Vector2us(1, 0), olddist, pDistance, bChanged);
				/*
				c = i+offset_l;
				cdistx = distx[c];
				cdisty = disty[c];
				newdistx = cdistx+1;
				newdisty = cdisty;
				newdist = DISTAA(c, cdistx, cdisty, newdistx, newdisty);
				if(newdist < olddist-epsilon)
				{
					distx[i]=newdistx;
					disty[i]=newdisty;
					pDistance[i]=newdist;
					bChanged = 1;
				}
				*/
			}
		}
	}
	while(bChanged); // Sweep until no more updates are made

	/* The transformation is completed. */
}

void GUIEclidianDistanceTransformAA::distanceMap(double* pImage, const size_t szWidth, const size_t szHeight)
{
	MouCa::assertion(m_pImage!=nullptr);
	MouCa::assertion(szWidth > 1 && szHeight > 1);

	//Copy all information
	m_szWidth		= szWidth;
	m_szHeight		= szHeight;
	m_MemorySize	= szWidth*szHeight;
	m_pImage		= pImage;

	m_pGradiantPts = new RT::Vector2d[m_MemorySize];

	RT::Vector2us*	pDistancePts	= new RT::Vector2us[m_MemorySize];	
	double*				pOutside		= new double[m_MemorySize];
	double*				pInside			= new double[m_MemorySize];

	// Compute outside = edtaa3(bitmap); % Transform background (0's)
	ComputeGradient(m_pGradiantPts);
	edtaa3(pDistancePts, pOutside);
	for(size_t szPixel=0; szPixel<m_MemorySize; ++szPixel)
	{
		if(pOutside[szPixel] < 0.0)
		{
			pOutside[szPixel] = 0.0;
		}
	}

	// Compute inside = edtaa3(1-bitmap); % Transform foreground (1's)
//    memset( gx, 0, sizeof(double)*szWidth*szHeight );
//    memset( gy, 0, sizeof(double)*szWidth*szHeight );
	for(size_t szPixel=0; szPixel<m_MemorySize; ++szPixel)
	{
		m_pImage[szPixel] = 1 - m_pImage[szPixel];
	}
	ComputeGradient(m_pGradiantPts);
	edtaa3(pDistancePts, pInside);
	for(size_t szPixel=0; szPixel<m_MemorySize; ++szPixel)
	{
		if(pInside[szPixel] < 0)
		{
			pInside[szPixel] = 0.0;
		}
	}

	// distmap = outside - inside; % Bipolar distance field
	double vmin = std::numeric_limits<double>::infinity();
	for(size_t szPixel=0; szPixel<m_MemorySize; ++szPixel)
	{
		pOutside[szPixel] -= pInside[szPixel];
		if(pOutside[szPixel] < vmin)
		{
			vmin = pOutside[szPixel];
		}
	}
	delete [] pInside; //Release at this time
	vmin = abs(vmin);
	MouCa::assertion(vmin != std::numeric_limits<double>::infinity());

	const double d1_2Vmin = 1.0/(2.0*vmin);
	for(size_t szPixel=0; szPixel<m_MemorySize; ++szPixel)
	{
        pOutside[szPixel] = std::clamp(pOutside[szPixel], -vmin, +vmin);
		m_pImage[szPixel] = (pOutside[szPixel]+vmin)*d1_2Vmin; 
	}

	delete [] m_pGradiantPts;
	delete [] pDistancePts;
	delete [] pOutside;
}