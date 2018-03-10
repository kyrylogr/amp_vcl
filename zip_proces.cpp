//---------------------------------------------------------------------------

#pragma hdrstop

#include "zip_proces.h"
#include "module_performer.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

CAmpZipCallback::CAmpZipCallback(int* b_abort_flag,progress_interface* pi) :
		m_p_abort_flag(b_abort_flag),
		m_pi(pi),m_was_breaked(false)
{
	m_iType = cbAll;
}

bool CAmpZipCallback::Callback(ZIP_SIZE_TYPE  iProgress)
{
	bool bContinue = !m_p_abort_flag || (!*m_p_abort_flag);

	if (bContinue && m_pi) {
		if (m_iType == cbCalculateForMulti)
		{
			bContinue = m_pi->progress(100, 0, "...");
		} else {
			unsigned int toProcess =  m_uTotalToProcess;
			unsigned int processed =  m_uProcessed;
			CMultiActionsInfo* pMulti = GetMultiActionsInfo();
			if (pMulti) {
				toProcess = pMulti->m_uTotalBytesToProcess;
				processed = pMulti->m_uBytesProcessed;
			}
			if (!toProcess) {
				toProcess = 100;
				processed = 0;
			} else {
				while (toProcess>200) {
					toProcess = toProcess/10;
					processed = processed/10;
				}
			}
			bContinue = m_pi->progress(toProcess, processed, (LPCTSTR)m_szFileInZip);
		}
	}
	if (!bContinue) {
		m_was_breaked = true;
	}
	return bContinue;
}

void zip_operation(String source, String target, int izlevel, bool btimestamp, int* b_abort_flag, progress_interface* pi)
{
	try{
	//
		CAmpZipCallback zcb(b_abort_flag, pi);
		int nLenSource = source.Length();
		int iStartFilePos = 1;
		std::vector<String> v;
		int z=1;
		for ( ; z <= nLenSource; z++) {
			String sourceItem;
			if (source[z]==';') {
				String sourceItem = source.SubString(iStartFilePos, z- iStartFilePos);
				v.push_back(sourceItem);
				iStartFilePos = z+1;
			}
		}
		if (iStartFilePos < z) {
			String sourceItem = source.SubString(iStartFilePos, z- iStartFilePos);
			v.push_back(sourceItem);
		}

		for (unsigned int i=0; i < v.size(); i++) {
			String sourceItem = v[i];
			sourceItem = ExcludeTrailingBackslash(sourceItem);
			String sfileName;
			bool bsourceIsDirectory = DirectoryExists(sourceItem);
			bool bsourceIsFile = !bsourceIsDirectory && FileExists(sourceItem);
			if (!bsourceIsDirectory && !bsourceIsFile) {
				throw EFOpenError("Source file or directory is not found: %s", ARRAYOFCONST( ( sourceItem) ) );
			}
			if (DirectoryExists(target)) {
				sfileName = ExtractFileName(sourceItem);
				//   check if source is file or directory;
				if (bsourceIsFile) {
					int nExt = ExtractFileExt(sfileName).Length();
					if (nExt) {
						sfileName.SetLength(sfileName.Length()-nExt);
					}
				}
				if (btimestamp) {
					String sstamp =  Now().FormatString("mmddyyyy");
					sfileName += sstamp;
				}
				target = IncludeTrailingBackslash(target) + sfileName + ".zip";
			} else {
				String sTargetDir = ExtractFilePath(sourceItem);
				if (!sTargetDir.IsEmpty() && !DirectoryExists(sTargetDir)) {
					throw EFOpenError("Target directory does not exist: %s", ARRAYOFCONST( (sTargetDir) ));
				}
			}
	//		bool isFile = !PathIsDirectory(target);
//			zip_create_process(sourceItem, target, izlevel, &zcb);
			CZipArchive     zip;
			zip.SetCallback(&zcb );
			zip.Open(target.c_str(), CZipArchive::zipCreate);
			if (bsourceIsFile) {
				zip.AddNewFile(sourceItem.c_str(), izlevel, false);
			} else {
				String sRootPath = ExtractFilePath(sourceItem);
				if (!sRootPath.IsEmpty()) {
					zip.SetRootPath(sRootPath.c_str());
				}
				zip.AddNewFiles(sourceItem.c_str(), _T("*.*"), true, izlevel, false);
			}
			zip.Close();
		}
	} catch (CZipException& e) {
//		if (m_was_breaked) {
//			throw AmpBreakException();
//		}
		throw Exception((LPCTSTR)e.GetErrorDescription() );
	}
}
