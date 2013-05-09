// Indexing functions with MATLAB semantic
// Useful for actual implementations, but also for analyses ("interpretation")
// Everything is column-major (or last-to-first for ND array)

namespace mcvm { namespace indexing {
    
template <typename T, typename V>
void setElem(const T& indices, const T& size, V& vector) {
    auto itr = indices.begin() ;
    auto index = *(itr++);
    
		// Initialize the dimension iterator
		DimVector::const_iterator dimItr = m_size.begin();
		
		// Initialize the offset product
		size_t offset = 1;
		
		// For each of the indices
		while (indItr != indices.end())
		{
			// Extract the size of the current dimension
			size_t dimSize = *(dimItr++);
			
			// Update the offset product
			offset *= dimSize;
			
			// Extract the index along the current dimension
			size_t dimIndex = *(indItr++);					
			
			// Convert the dimensional index to zero indexing
			dimIndex = toZeroIndex(dimIndex);
			
			// Update the index sum
			index += dimIndex * offset;
		}

		// Ensure that the global index is valid
		assert (index < m_numElements);
		
		// Set the desired element
		m_pElements[index] = value;
	}

	void setElem2D(size_t rowIndex, size_t colIndex, ScalarType value)
	{
		// Convert to zero indexing
		rowIndex = toZeroIndex(rowIndex);
		colIndex = toZeroIndex(colIndex);		
		
		// Ensure the indices are valid
		assert (m_numElements > 0 && rowIndex < m_size[0] && colIndex < m_size[1]);
		
		// Compute the element index
		size_t index = rowIndex + colIndex * m_size[0];
		
		// Ensure the index is valid
		assert (index < m_numElements);
		
		// Set the desired element
		m_pElements[index] = value;
	}
}

}}
