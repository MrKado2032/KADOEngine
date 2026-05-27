#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>
#include <stdexcept>
#include <cstring>

#include "GraphicsKernel.h"

template<typename T>
class Buffer
{
public:
	Buffer(const std::vector<T>& data, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	~Buffer();

	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

	Buffer(Buffer&& other) noexcept;
	Buffer& operator=(Buffer&& other) noexcept;

	void Upload(const std::vector<T>& data);

private:
	void Destroy();

	VkBuffer m_buffer = VK_NULL_HANDLE;
	VkDeviceMemory m_deviceMemory = VK_NULL_HANDLE;

	VkDeviceSize m_deviceSize = 0;
};

template<typename T>
inline Buffer<T>::Buffer(const std::vector<T>& data, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
{
	if (data.empty())
	{
		throw std::runtime_error("buffer is data empty");
	}

	const auto device = GraphicsKernel::GetDevice();

	VkBufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.flags = 0;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.size = m_deviceSize = sizeof(T) * data.size();
	bufferCreateInfo.usage = usage;

	if (vkCreateBuffer(device, &bufferCreateInfo, nullptr, &m_buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, m_buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = GraphicsKernel::FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &m_deviceMemory) != VK_SUCCESS)
	{
		Destroy();
		throw std::runtime_error("failed to allocate memory");
	}

	if (vkBindBufferMemory(device, m_buffer, m_deviceMemory, 0) != VK_SUCCESS)
	{
		Destroy();
		throw std::runtime_error("failed to bind buffer memory");
	}

	Upload(data);
}

template<typename T>
inline Buffer<T>::~Buffer()
{
	Destroy();
}

template<typename T>
inline Buffer<T>::Buffer(Buffer&& other) noexcept
{
	m_buffer = other.m_buffer;
	m_deviceMemory = other.m_deviceMemory;
	m_deviceSize = other.m_deviceSize;

	other.m_buffer = VK_NULL_HANDLE;
	other.m_deviceMemory = VK_NULL_HANDLE;
	other.m_deviceSize = 0;
}

template<typename T>
inline Buffer<T>& Buffer<T>::operator=(Buffer&& other) noexcept
{
	if (this != &other)
	{
		Destroy();

		m_buffer = other.m_buffer;
		m_deviceMemory = other.m_deviceMemory;
		m_deviceSize = other.m_deviceSize;

		other.m_buffer = VK_NULL_HANDLE;
		other.m_deviceMemory = VK_NULL_HANDLE;
		other.m_deviceSize = 0;
	}

	return *this;
}

template<typename T>
inline void Buffer<T>::Upload(const std::vector<T>& data)
{
	if (data.empty())
	{
		throw std::runtime_error("upload data is empty");
	}

	const auto device = GraphicsKernel::GetDevice();

	const VkDeviceSize uploadSize = sizeof(T) * data.size();
	if (uploadSize > m_deviceSize)
	{
		throw std::runtime_error("upload data is larger than buffer size");
	}

	void* mappedData = nullptr;
	if (vkMapMemory(device, m_deviceMemory, 0, uploadSize, 0, &mappedData) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to map buffer memory");
	}
	std::memcpy(mappedData, data.data(), static_cast<size_t>(uploadSize));
	vkUnmapMemory(device, m_deviceMemory);
}

template<typename T>
inline void Buffer<T>::Destroy()
{
	const auto device = GraphicsKernel::GetDevice();

	if (m_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(device, m_buffer, nullptr);
		m_buffer = VK_NULL_HANDLE;
	}

	if (m_deviceMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(device, m_deviceMemory, nullptr);
		m_deviceMemory = VK_NULL_HANDLE;
	}

	m_deviceSize = 0;
}
