mkdir runtime_tests64
cd runtime_tests64
copy \\ismblcr\nfs\iil\disks\cvcc\OclConformance\trunk\ReleaseCriteria\Win64_Release.7z Win64_Release.7z
copy \\ismblcr\nfs\iil\disks\cvcc\OclConformance\trunk\ReleaseCriteria\Workloads.7z Workloads.7z

"C:\Program Files\7-Zip\7z.exe" x Win64_Release.7z
"C:\Program Files\7-Zip\7z.exe" x Workloads.7z