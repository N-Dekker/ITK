#-- # Grading Level Criteria Report
#-- EVALUATION DATE: 2020-03-24
#-- EVALUATORS: [Dženan Zukić]
#--
#-- ## Compliance level 5 star (AKA ITK main modules, or remote modules that could become core modules)
#--   - [ ] Widespread community dependance
#--   - [ ] Above 90% code coverage
#--   - [ ] CI dashboards and testing monitored rigorously
#--   - [X] Key API features are exposed in wrapping interface
#--   - [ ] All requirements of Levels 4,3,2,1
#--
#-- ## Compliance Level 4 star (Very high-quality code, perhaps small community dependance)
#--   - [X] Meets all ITK code style standards
#--   - [X] No external requirements beyond those needed by ITK proper
#--   - [ ] Builds and passes tests on all supported platforms within 1 month of each core tagged release
#--            - [ ] Windows Shared Library Build with Visual Studio
#--            - [ ] Mac with clang compiller
#--            - [ ] Linux with gcc compiler
#--   - [ ] Active developer community dedicated to maintaining code-base
#--   - [ ] 75% code coverage demonstrated for testing suite
#--   - [X] Continuous integration testing performed
#--   - [X] All requirements of Levels 3,2,1
#--
#-- ## Compliance Level 3 star (Quality beta code)
#--   - [X] API | executable interface is considered mostly stable and feature complete
#--   - [X] 10% C0-code coverage demonstrated for testing suite
#--   - [X] Some tests exist and pass on at least some platform
#--   - [X] All requirements of Levels 2,1
#--
#-- ## Compliance Level 2 star (Alpha code feature API development or niche community/execution environment dependance )
#--   - [X] Compiles for at least 1 niche set of execution envirionments, and perhaps others
#--         (may depend on specific external tools like a java environment, or specific external libraries to work )
#--   - [X] All requirements of Levels 1
#--
#-- ## Compliance Level 1 star (Pre-alpha features under development and code of unknown quality)
#--   - [X] Code complies on at least 1 platform
#--
#-- ## Compliance Level 0 star ( Code/Feature of known poor-quality or deprecated status )
#--   - [ ] Code reviewed and explicitly identified as not recommended for use
#--
#-- ### Please document here any justification for the criteria above
#       Code style enforced by clang-format on 2020-02-19, and clang-tidy modernizations completed

itk_fetch_module(BoneMorphometry
  "An Insight Toolkit (ITK) based implementation for bone morphometry quantification from computed tomography (CT) images.
  A more detailed description can be found in the Insight Journal (IJ) article:
  Vimort J., McCormick M., Paniagua B.
  Computing Bone Morphometric Feature Maps from 3-Dimensional Images
  The Insight Journal. January-December. 2017.
  https://www.insight-journal.org/browse/publication/988
  "
  MODULE_COMPLIANCE_LEVEL 3
  GIT_REPOSITORY ${git_protocol}://github.com/InsightSoftwareConsortium/ITKBoneMorphometry.git
  GIT_TAG 47b8f352fb2cb7204d7a8b332847da438f268b35
)
