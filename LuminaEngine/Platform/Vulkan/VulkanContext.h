//
// Created by Faith Kamaraju on 2026-01-18.
//

#pragma once
#include <vulkan/vulkan.hpp>
#include <VmaUsage.h>
#include "QueueFamilyIndices.h"
#include "VulkanUtils.h"
#include "Descriptors.h"


namespace LE {

    struct VulkanContext {
        vk::Instance instance = nullptr;
        vk::DebugUtilsMessengerEXT debugMessenger = nullptr;
        vk::SurfaceKHR surface = nullptr;
        vk::PhysicalDevice primaryPhysicalDevice = nullptr;
        std::multimap<int, vk::PhysicalDevice> availablePhysicalDevices;
        std::multimap<int, QueueFamilyIndices> availablePhysicalDevicesQueueIndices;
        QueueFamilyIndices queueFamilyIndices;
        std::vector<uint32_t> queueIndicesArr;
        vk::Device device = nullptr;
        VmaAllocator vmaAllocator = nullptr;
        vk::PipelineCache pipelineCache = nullptr;
        vk::Queue graphicsQueue = nullptr;
        vk::Queue presentQueue = nullptr;
        vk::Queue transferQueue = nullptr;
        bool bUnifiedGraphicsAndTransferQueues = false;
        vk::CommandPool globalGraphicsCmdPool = nullptr;
        vk::CommandPool transferCommandPool = nullptr;
        bool bUseStagingBuffer = true;
        DescriptorAllocator descriptorAllocator;

        // vk::PhysicalDeviceVulkan14Features Features14 = {};
        vk::PhysicalDeviceVulkan13Features features13 = {};
        vk::PhysicalDeviceVulkan12Features features12 = {.pNext = &features13};
        vk::PhysicalDeviceVulkan11Features features11 = {.pNext = &features12};
        vk::PhysicalDeviceFeatures2 features10 = {.pNext = &features11};

        // vk::PhysicalDeviceVulkan14Features   PhysicalDeviceVulkan14Props = {};
        vk::PhysicalDeviceVulkan13Properties physicalDeviceVulkan13Properties = {};
        vk::PhysicalDeviceVulkan12Properties physicalDeviceVulkan12Properties = {.pNext = &physicalDeviceVulkan13Properties,};
        vk::PhysicalDeviceVulkan11Properties physicalDeviceVulkan11Properties = {.pNext = &physicalDeviceVulkan12Properties,};
        vk::PhysicalDeviceProperties2 physicalDeviceProperties2 = {
            .pNext = &physicalDeviceVulkan11Properties,
            .properties = {}

        };

        inline void InitVulkanInstance() {

             vk::detail::DynamicLoader dl;
            auto vkGetinstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
            VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetinstanceProcAddr);

            constexpr vk::ApplicationInfo appInfo{
                .pApplicationName = "LuminaGame",
                .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
                .pEngineName = "LuminaEngine",
                .engineVersion = VK_MAKE_VERSION(0, 0, 1),
                .apiVersion = vk::ApiVersion13
            };

            const std::vector<const char *> reqExtensions = Utils::GetRequiredExtensions();

            vk::InstanceCreateInfo createInfo{
                .flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR,
                .pApplicationInfo = &appInfo,
                .enabledExtensionCount = static_cast<uint32_t>(reqExtensions.size()),
                .ppEnabledExtensionNames = reqExtensions.data(),
            };
            if (Utils::enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(Utils::instanceLayers.size());
                createInfo.ppEnabledLayerNames = Utils::instanceLayers.data();
            }

                // #ifdef LE_PLATFORM_MAC
                //     Utils::createMoltenVkLayerSettings(createInfo);
                // #endif

            instance = vk::createInstance(createInfo, nullptr);
            LE_CORE_INFO("Vulkan instance successfully created!");
            VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

            if (Utils::enableValidationLayers && !Utils::CheckValidationLayerSupport()) {
                throw std::runtime_error("validation layers requested, but not available!");
            }
            if (Utils::enableValidationLayers) {
                vk::DebugUtilsMessengerCreateInfoEXT messengerCreateInfo{
                    .messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
                                       | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                                       | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
                    .messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                                   | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
                                   | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
                    .pfnUserCallback = &Utils::debugCallback
                };
                debugMessenger = instance.createDebugUtilsMessengerEXT(messengerCreateInfo, nullptr);
            }

        }

        inline void AcquireSurface(GLFWwindow *windowHandle) {

            VkSurfaceKHR surf;
            if (glfwCreateWindowSurface(instance, windowHandle, nullptr, &surf) != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface!");
            }
            surface = vk::SurfaceKHR(surf);
            LE_CORE_INFO("Acquired Surface!");
        }

        void PickPhysicalDevice() {

            auto physicaldevices = instance.enumeratePhysicalDevices();

            LE_CORE_INFO("Number of physical devices found: {0}", physicaldevices.size());
            if (!physicaldevices.size()) {
                throw std::runtime_error("Couldn't find any Vulkan Capable devices!");
            }
            for (const auto& device : physicaldevices) {

                int score = Utils::ScoreDevices(device);

                QueueFamilyIndices queueIndices = Utils::CheckQueueFamilySupport(device, surface);
                if (!queueIndices.isComplete()) score = 0 ;

                bool extensionsSupported = Utils::CheckDeviceExtensionsSupport(device);
                if (!extensionsSupported) score = 0;

                bool swapChainAdequate = false;
                if (extensionsSupported) {
                    SwapChainSupportDetails details = Utils::QuerySwapChainSupportDetails(device, surface);
                    swapChainAdequate = !details.formats.empty() && !details.presentModes.empty();
                }

                if (!swapChainAdequate) score = 0;

                availablePhysicalDevices.insert(std::make_pair(score, device));
                availablePhysicalDevicesQueueIndices.insert(std::make_pair(score, queueIndices));
            }
            if (availablePhysicalDevices.rbegin()->first > 0) {
                primaryPhysicalDevice = availablePhysicalDevices.rbegin()->second;
                queueFamilyIndices =  availablePhysicalDevicesQueueIndices.rbegin()->second;
                if (queueFamilyIndices.graphicsFamily.value() == queueFamilyIndices.transferFamily.value()) {
                    bUnifiedGraphicsAndTransferQueues = true;
                }
            } else {
                throw std::runtime_error("failed to find a suitable GPU!");
            }
            // Vulkan::Utils::CheckForUnifiedMemory(this, primaryPhysicalDevice);
            LE_CORE_INFO("Physical device Chosen: {0}", primaryPhysicalDevice.getProperties().deviceName.data());
        }

        void CreateLogicalDevice() {

            std::set uniqueQueueFamilies = {queueFamilyIndices.graphicsFamily.value(),
                                            queueFamilyIndices.presentFamily.value(),
                                            queueFamilyIndices.transferFamily.value()};

            std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

            float queuePriority = 1.0f;
            for (uint32_t queueFamily : uniqueQueueFamilies) {
                vk::DeviceQueueCreateInfo queueCreateInfo{
                .queueFamilyIndex = queueFamily,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority,
                };
                queueCreateInfos.push_back(queueCreateInfo);
            }

            features10 = primaryPhysicalDevice.getFeatures2();
            physicalDeviceProperties2 = primaryPhysicalDevice.getProperties2();

            vk::PhysicalDeviceFeatures deviceFeatures10 = {
              .geometryShader = features10.features.geometryShader, // enable if supported
              .tessellationShader = features10.features.tessellationShader, // enable if supported
              .sampleRateShading = VK_TRUE,
              .multiDrawIndirect = VK_TRUE,
              .drawIndirectFirstInstance = VK_TRUE,
              .depthBiasClamp = VK_TRUE,
              .fillModeNonSolid = features10.features.fillModeNonSolid, // enable if supported
              .samplerAnisotropy = VK_TRUE,
              .textureCompressionBC = features10.features.textureCompressionBC, // enable if supported
              .vertexPipelineStoresAndAtomics = features10.features.vertexPipelineStoresAndAtomics, // enable if supported
              .fragmentStoresAndAtomics = VK_TRUE,
              .shaderImageGatherExtended = VK_TRUE,
              .shaderInt64 = features10.features.shaderInt64, // enable if supported
              .shaderInt16 = features10.features.shaderInt16, // enable if supported
            };
            vk::PhysicalDeviceVulkan11Features deviceFeatures11 = {
              .pNext = nullptr,
              .storageBuffer16BitAccess = VK_TRUE,
              .multiview = features11.multiview, // enable if supported
              .samplerYcbcrConversion = features11.samplerYcbcrConversion, // enable if supported
              .shaderDrawParameters = VK_TRUE,
            };
            vk::PhysicalDeviceVulkan12Features deviceFeatures12 = {
              .pNext = &deviceFeatures11,
              .drawIndirectCount = features12.drawIndirectCount, // enable if supported
              .storageBuffer8BitAccess = features12.storageBuffer8BitAccess, // enable if supported
              .uniformAndStorageBuffer8BitAccess = features12.uniformAndStorageBuffer8BitAccess, // enable if supported
              .shaderFloat16 = features12.shaderFloat16, // enable if supported
              .shaderInt8 = features12.shaderInt8, // enable if supported
              .descriptorIndexing = VK_TRUE,
              .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
              .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
              .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
              .descriptorBindingUpdateUnusedWhilePending = VK_TRUE,
              .descriptorBindingPartiallyBound = VK_TRUE,
              .descriptorBindingVariableDescriptorCount = VK_TRUE,
              .runtimeDescriptorArray = VK_TRUE,
              .scalarBlockLayout = VK_TRUE,
              .uniformBufferStandardLayout = VK_TRUE,
              .hostQueryReset = features12.hostQueryReset, // enable if supported
              .timelineSemaphore = VK_TRUE,
              .bufferDeviceAddress = VK_TRUE,
              .vulkanMemoryModel = features12.vulkanMemoryModel, // enable if supported
              .vulkanMemoryModelDeviceScope = features12.vulkanMemoryModelDeviceScope, // enable if supported
            };
            vk::PhysicalDeviceVulkan13Features deviceFeatures13 = {
              .pNext = &deviceFeatures12,
              .shaderDemoteToHelperInvocation = features13.shaderDemoteToHelperInvocation, // enable if supported
              .subgroupSizeControl = VK_TRUE,
              .synchronization2 = VK_TRUE,
              .dynamicRendering = VK_TRUE,
              .maintenance4 = VK_TRUE,
            };

            std::string missingFeatures;
            #define CHECK_VULKAN_FEATURE(reqFeatures, availFeatures, feature, version)     \
              if ((reqFeatures.feature == VK_TRUE) && (availFeatures.feature == VK_FALSE)) \
                missingFeatures.append("\n   " version " ." #feature);
            #define CHECK_FEATURE_1_0(feature) CHECK_VULKAN_FEATURE(deviceFeatures10, features10.features, feature, "1.0 ");
                CHECK_FEATURE_1_0(robustBufferAccess);
                CHECK_FEATURE_1_0(fullDrawIndexUint32);
                CHECK_FEATURE_1_0(imageCubeArray);
                CHECK_FEATURE_1_0(independentBlend);
                CHECK_FEATURE_1_0(geometryShader);
                CHECK_FEATURE_1_0(tessellationShader);
                CHECK_FEATURE_1_0(sampleRateShading);
                CHECK_FEATURE_1_0(dualSrcBlend);
                CHECK_FEATURE_1_0(logicOp);
                CHECK_FEATURE_1_0(multiDrawIndirect);
                CHECK_FEATURE_1_0(drawIndirectFirstInstance);
                CHECK_FEATURE_1_0(depthClamp);
                CHECK_FEATURE_1_0(depthBiasClamp);
                CHECK_FEATURE_1_0(fillModeNonSolid);
                CHECK_FEATURE_1_0(depthBounds);
                CHECK_FEATURE_1_0(wideLines);
                CHECK_FEATURE_1_0(largePoints);
                CHECK_FEATURE_1_0(alphaToOne);
                CHECK_FEATURE_1_0(multiViewport);
                CHECK_FEATURE_1_0(samplerAnisotropy);
                CHECK_FEATURE_1_0(textureCompressionETC2);
                CHECK_FEATURE_1_0(textureCompressionASTC_LDR);
                CHECK_FEATURE_1_0(textureCompressionBC);
                CHECK_FEATURE_1_0(occlusionQueryPrecise);
                CHECK_FEATURE_1_0(pipelineStatisticsQuery);
                CHECK_FEATURE_1_0(vertexPipelineStoresAndAtomics);
                CHECK_FEATURE_1_0(fragmentStoresAndAtomics);
                CHECK_FEATURE_1_0(shaderTessellationAndGeometryPointSize);
                CHECK_FEATURE_1_0(shaderImageGatherExtended);
                CHECK_FEATURE_1_0(shaderStorageImageExtendedFormats);
                CHECK_FEATURE_1_0(shaderStorageImageMultisample);
                CHECK_FEATURE_1_0(shaderStorageImageReadWithoutFormat);
                CHECK_FEATURE_1_0(shaderStorageImageWriteWithoutFormat);
                CHECK_FEATURE_1_0(shaderUniformBufferArrayDynamicIndexing);
                CHECK_FEATURE_1_0(shaderSampledImageArrayDynamicIndexing);
                CHECK_FEATURE_1_0(shaderStorageBufferArrayDynamicIndexing);
                CHECK_FEATURE_1_0(shaderStorageImageArrayDynamicIndexing);
                CHECK_FEATURE_1_0(shaderClipDistance);
                CHECK_FEATURE_1_0(shaderCullDistance);
                CHECK_FEATURE_1_0(shaderFloat64);
                CHECK_FEATURE_1_0(shaderInt64);
                CHECK_FEATURE_1_0(shaderInt16);
                CHECK_FEATURE_1_0(shaderResourceResidency);
                CHECK_FEATURE_1_0(shaderResourceMinLod);
                CHECK_FEATURE_1_0(sparseBinding);
                CHECK_FEATURE_1_0(sparseResidencyBuffer);
                CHECK_FEATURE_1_0(sparseResidencyImage2D);
                CHECK_FEATURE_1_0(sparseResidencyImage3D);
                CHECK_FEATURE_1_0(sparseResidency2Samples);
                CHECK_FEATURE_1_0(sparseResidency4Samples);
                CHECK_FEATURE_1_0(sparseResidency8Samples);
                CHECK_FEATURE_1_0(sparseResidency16Samples);
                CHECK_FEATURE_1_0(sparseResidencyAliased);
                CHECK_FEATURE_1_0(variableMultisampleRate);
                CHECK_FEATURE_1_0(inheritedQueries);
            #undef CHECK_FEATURE_1_0
            #define CHECK_FEATURE_1_1(feature) CHECK_VULKAN_FEATURE(deviceFeatures11, features11, feature, "1.1 ");
                CHECK_FEATURE_1_1(storageBuffer16BitAccess);
                CHECK_FEATURE_1_1(uniformAndStorageBuffer16BitAccess);
                CHECK_FEATURE_1_1(storagePushConstant16);
                CHECK_FEATURE_1_1(storageInputOutput16);
                CHECK_FEATURE_1_1(multiview);
                CHECK_FEATURE_1_1(multiviewGeometryShader);
                CHECK_FEATURE_1_1(multiviewTessellationShader);
                CHECK_FEATURE_1_1(variablePointersStorageBuffer);
                CHECK_FEATURE_1_1(variablePointers);
                CHECK_FEATURE_1_1(protectedMemory);
                CHECK_FEATURE_1_1(samplerYcbcrConversion);
                CHECK_FEATURE_1_1(shaderDrawParameters);
            #undef CHECK_FEATURE_1_1
            #define CHECK_FEATURE_1_2(feature) CHECK_VULKAN_FEATURE(deviceFeatures12, features12, feature, "1.2 ");
                CHECK_FEATURE_1_2(samplerMirrorClampToEdge);
                CHECK_FEATURE_1_2(drawIndirectCount);
                CHECK_FEATURE_1_2(storageBuffer8BitAccess);
                CHECK_FEATURE_1_2(uniformAndStorageBuffer8BitAccess);
                CHECK_FEATURE_1_2(storagePushConstant8);
                CHECK_FEATURE_1_2(shaderBufferInt64Atomics);
                CHECK_FEATURE_1_2(shaderSharedInt64Atomics);
                CHECK_FEATURE_1_2(shaderFloat16);
                CHECK_FEATURE_1_2(shaderInt8);
                CHECK_FEATURE_1_2(descriptorIndexing);
                CHECK_FEATURE_1_2(shaderInputAttachmentArrayDynamicIndexing);
                CHECK_FEATURE_1_2(shaderUniformTexelBufferArrayDynamicIndexing);
                CHECK_FEATURE_1_2(shaderStorageTexelBufferArrayDynamicIndexing);
                CHECK_FEATURE_1_2(shaderUniformBufferArrayNonUniformIndexing);
                CHECK_FEATURE_1_2(shaderSampledImageArrayNonUniformIndexing);
                CHECK_FEATURE_1_2(shaderStorageBufferArrayNonUniformIndexing);
                CHECK_FEATURE_1_2(shaderStorageImageArrayNonUniformIndexing);
                CHECK_FEATURE_1_2(shaderInputAttachmentArrayNonUniformIndexing);
                CHECK_FEATURE_1_2(shaderUniformTexelBufferArrayNonUniformIndexing);
                CHECK_FEATURE_1_2(shaderStorageTexelBufferArrayNonUniformIndexing);
                CHECK_FEATURE_1_2(descriptorBindingUniformBufferUpdateAfterBind);
                CHECK_FEATURE_1_2(descriptorBindingSampledImageUpdateAfterBind);
                CHECK_FEATURE_1_2(descriptorBindingStorageImageUpdateAfterBind);
                CHECK_FEATURE_1_2(descriptorBindingStorageBufferUpdateAfterBind);
                CHECK_FEATURE_1_2(descriptorBindingUniformTexelBufferUpdateAfterBind);
                CHECK_FEATURE_1_2(descriptorBindingStorageTexelBufferUpdateAfterBind);
                CHECK_FEATURE_1_2(descriptorBindingUpdateUnusedWhilePending);
                CHECK_FEATURE_1_2(descriptorBindingPartiallyBound);
                CHECK_FEATURE_1_2(descriptorBindingVariableDescriptorCount);
                CHECK_FEATURE_1_2(runtimeDescriptorArray);
                CHECK_FEATURE_1_2(samplerFilterMinmax);
                CHECK_FEATURE_1_2(scalarBlockLayout);
                CHECK_FEATURE_1_2(imagelessFramebuffer);
                CHECK_FEATURE_1_2(uniformBufferStandardLayout);
                CHECK_FEATURE_1_2(shaderSubgroupExtendedTypes);
                CHECK_FEATURE_1_2(separateDepthStencilLayouts);
                CHECK_FEATURE_1_2(hostQueryReset);
                CHECK_FEATURE_1_2(timelineSemaphore);
                CHECK_FEATURE_1_2(bufferDeviceAddress);
                CHECK_FEATURE_1_2(bufferDeviceAddressCaptureReplay);
                CHECK_FEATURE_1_2(bufferDeviceAddressMultiDevice);
                CHECK_FEATURE_1_2(vulkanMemoryModel);
                CHECK_FEATURE_1_2(vulkanMemoryModelDeviceScope);
                CHECK_FEATURE_1_2(vulkanMemoryModelAvailabilityVisibilityChains);
                CHECK_FEATURE_1_2(shaderOutputViewportIndex);
                CHECK_FEATURE_1_2(shaderOutputLayer);
                CHECK_FEATURE_1_2(subgroupBroadcastDynamicId);
            #undef CHECK_FEATURE_1_2
            #define CHECK_FEATURE_1_3(feature) CHECK_VULKAN_FEATURE(deviceFeatures13, features13, feature, "1.3 ");
                CHECK_FEATURE_1_3(robustImageAccess);
                CHECK_FEATURE_1_3(inlineUniformBlock);
                CHECK_FEATURE_1_3(descriptorBindingInlineUniformBlockUpdateAfterBind);
                CHECK_FEATURE_1_3(pipelineCreationCacheControl);
                CHECK_FEATURE_1_3(privateData);
                CHECK_FEATURE_1_3(shaderDemoteToHelperInvocation);
                CHECK_FEATURE_1_3(shaderTerminateInvocation);
                CHECK_FEATURE_1_3(subgroupSizeControl);
                CHECK_FEATURE_1_3(computeFullSubgroups);
                CHECK_FEATURE_1_3(synchronization2);
                CHECK_FEATURE_1_3(textureCompressionASTC_HDR);
                CHECK_FEATURE_1_3(shaderZeroInitializeWorkgroupMemory);
                CHECK_FEATURE_1_3(dynamicRendering);
                CHECK_FEATURE_1_3(shaderIntegerDotProduct);
                CHECK_FEATURE_1_3(maintenance4);
            #undef CHECK_FEATURE_1_3

            if (missingFeatures.empty()) {
                LE_CORE_ERROR("Missing Vulkan Features! {0}", missingFeatures.c_str());
            }

            void* createInfoNext = &deviceFeatures13;
            vk::DeviceCreateInfo createInfo{};
            createInfo.pNext = createInfoNext;
            createInfo.pQueueCreateInfos = queueCreateInfos.data();
            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.enabledExtensionCount = static_cast<uint32_t>(Utils::deviceExtensions.size());
            createInfo.ppEnabledExtensionNames = Utils::deviceExtensions.data();
            createInfo.pEnabledFeatures = &deviceFeatures10;

            device = primaryPhysicalDevice.createDevice(createInfo, nullptr);
            LE_CORE_INFO("Logical device successfully created!");

            graphicsQueue = device.getQueue(queueFamilyIndices.graphicsFamily.value(),0);
            LE_CORE_INFO("Acquired the handle to graphics queue! Family Index = {0}", queueFamilyIndices.graphicsFamily.value());
            presentQueue = device.getQueue(queueFamilyIndices.presentFamily.value(), 0);
            LE_CORE_INFO("Acquired the handle to present queue! Family Index = {0}", queueFamilyIndices.presentFamily.value() );
            transferQueue = device.getQueue(queueFamilyIndices.transferFamily.value(), 0);
            LE_CORE_INFO("Acquired the handle to transfer queue! Family Index = {0}", queueFamilyIndices.transferFamily.value() );

        }

        void CreateVmaAllocator() {
            VmaVulkanFunctions vulkanFunctions = {};
            vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
            vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

            VmaAllocatorCreateInfo allocatorCreateInfo = {};
            allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT |
                                        VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT |
                                        VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT ;
            allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
            allocatorCreateInfo.physicalDevice = primaryPhysicalDevice;
            allocatorCreateInfo.device = device;
            allocatorCreateInfo.instance = instance;
            allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

            vmaCreateAllocator(&allocatorCreateInfo, &vmaAllocator);
            LE_CORE_INFO("VMA Allocator created!");
        }

        void InitPools() {
            vk::CommandPoolCreateInfo gpoolInfo{
                .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value(),
            };

            globalGraphicsCmdPool = device.createCommandPool(gpoolInfo, nullptr);

            vk::CommandPoolCreateInfo tpoolInfo{
                .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                .queueFamilyIndex = queueFamilyIndices.transferFamily.value(),
            };
            transferCommandPool = device.createCommandPool(tpoolInfo, nullptr);
        }

    };

}