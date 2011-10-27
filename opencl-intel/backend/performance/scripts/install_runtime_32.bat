mkdir runtime_tests32
cd runtime_tests32
copy \\ismblcr\nfs\iil\disks\cvcc\OclConformance\trunk\ReleaseCriteria\Win32_Release.7z Win32_Release.7z
copy \\ismblcr\nfs\iil\disks\cvcc\OclConformance\trunk\ReleaseCriteria\Workloads.7z Workloads.7z

"C:\Program Files\7-Zip\7z.exe" x Win32_Release.7z
"C:\Program Files\7-Zip\7z.exe" x Workloads.7z