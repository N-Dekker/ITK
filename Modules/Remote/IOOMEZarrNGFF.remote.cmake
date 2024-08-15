# TODO Niels Dekker, August 2024: A "Grading Level Criteria Report" may need to be added here.

itk_fetch_module(
  IOOMEZarrNGFF
  "ITK module for IO of images stored in Zarr-backed OME-NGFF file format."
  MODULE_COMPLIANCE_LEVEL 3
  GIT_REPOSITORY https://github.com/InsightSoftwareConsortium/ITKIOOMEZarrNGFF.git
  # commit 6bb50d2: "Merge pull request #64 from jhlegarreta/FixDuplicateTestOutputfilename", June 25, 2024
  GIT_TAG 6bb50d206670b2906558ebb47f77ad0e7306a7f0
  )
