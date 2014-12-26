/*++

Copyright (c) Ken Johnson (Skywing). All rights reserved.

Module Name:

	ResourceAccessor.cs

Abstract:

    This module houses the local resource access provider for the toolset
    internal resource system.  It is used to service requests by the script
    compiler when resources must be demanded (i.e. to handle script icnludes).

--*/

// #define DBGRESOURCE
// #define DUMPRESLIST

using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Runtime.InteropServices;
using OEIShared.IO;
using OEIShared.Utils;

namespace NWN2ToolsetCompilerPlugin
{

    [StructLayout(LayoutKind.Sequential)]
    public struct ResRef32
    {
        [MarshalAsAttribute(UnmanagedType.ByValArray, SizeConst = 32)]
        public byte[] RefStr;
    }

    public enum ResTypes : ushort
    {
        ResNSS = 2009,
        ResGFF = 2037,

        ResINVALID = ushort.MaxValue
    }

    //
    // Define the IResourceAccessor interface, which mirrosrs the standard
    // ResourceManager IResourceAccessor<NWN::ResRef32>-style interface.
    //

    public interface IResourceAccessor
    {
        //
        // Open an encapsulated file by resref.
        //

        ulong OpenFile(string ResRef, ushort ResType);

        //
        // Open an encapsulated file by file index.
        //

        ulong OpenFileByIndex(ulong FileIndex);

        //
        // Close an encapsulated file.
        //

        bool CloseFile(ulong FileHandle);

        //
        // Read an encapsulated file by handle.  The routine is optimized to
        // operate for sequential file reads.
        //

        bool ReadEncapsulatedFile(ulong File, IntPtr Offset, IntPtr BytesToRead, out IntPtr BytesRead, byte[] Buffer);

        //
        // Return the size of a file.
        //

        IntPtr GetEncapsulatedFileSize(ulong File);

        //
        // Return the resource type of a file.
        //

        ushort GetEncapsulatedFileType(ulong File);

        //
        // Iterate through resources in this resource accessor.  The routine
        // return false on failure.
        //

        bool GetEncapsulatedFileEntry(ulong FileIndex, out string ResRef, out ushort ResType);

        //
        // Return the count of encapsulated files in this accessor.
        //

        ulong GetEncapsulatedFileCount();

    }

    //
    // Define the ResourceAccessor object, which implements a ResourceManager
    // IResourceAccessor<NWN::ResRef32>-style interface against the toolset's
    // internal resource manager.
    //

    public class ResourceAccessor : IResourceAccessor
    {
        //
        // Construct a new ResourceAccessor.
        //

        public ResourceAccessor(NWN2ToolsetCompilerPlugin Plugin)
        {
            ResourceManager ResMan = ResourceManager.Instance;
            ResourceRepositoryCollection Repositories = ResMan.Repositories;

            m_Plugin = Plugin;

            m_ResourceEntries = new List<ResourceIndex>();
            m_NameIdMap = new Dictionary<ResourceKey, int>();
            m_OpenFileHandles = new Dictionary<ulong, ResHandle>();

            m_IndexValid = false;
            m_IndexInvalidPending = true;

            m_NextFileHandle = 0;

            //
            // Register to be invoked when resource repositories are changed,
            // or new repositories become available (that we'll have to
            // subscribe to).
            //

            Repositories.Changed += this.OnResourceRepositoryInvalidated;
            Repositories.Inserted += this.OnResourceRepositoryAdded;

            foreach (IResourceRepository ToolsetRepository in Repositories)
            {
                ToolsetRepository.Resources.Changed += this.OnResourceRepositoryContentsInvalidated;
            }
        }

        //
        // Ensure that the resource system is up to date, rescanning the
        // toolset resource collection if not.
        //

        public void EnsureIndexValid()
        {
            if (m_IndexValid)
                return;

            DiscoverResources();
        }

        //
        // Invalidate the internal index (perhaps because a new item was added
        // to the module).
        //

        public void InvalidateResourceIndexes()
        {
            m_IndexValid = false;
            m_IndexInvalidPending = true;
        }

        //
        // Check whether resource indexes have been invalidated since the last
        // acknowledgement.
        //

        public bool IsIndexInvalidationPending()
        {
            return m_IndexInvalidPending;
        }

        //
        // Acknowledge that the resource indexes have been invalidated.
        //

        public void AcknowledgeIndexInvalidation()
        {
            m_IndexInvalidPending = false;
        }

        //
        // IResourceAccessor.
        //

        public const ulong INVALID_FILE = 0;
        public const ushort ResINVALID = 0xFFFF;

        //
        // Open an encapsulated file by resref.
        //

        public virtual ulong OpenFile(string ResRef, ushort ResType)
        {
            ResourceKey SearchKey;
            int FileId;

            //
            // Look up the file and see if we've got a match.
            //

            SearchKey.Name = ResRef;
            SearchKey.Type = ResType;

            if (!m_NameIdMap.TryGetValue(SearchKey, out FileId))
                return INVALID_FILE;

            return OpenFileByIndex((ulong)FileId);
        }

        //
        // Open an encapsulated file by file index.
        //

        public virtual ulong OpenFileByIndex(ulong FileIndex)
        {
            ResHandle OpenFile;
            ulong FileHandle;
            int Idx = (int)FileIndex;

            //
            // Open a new stream to the file in question.
            //

            if (Idx < 0 || Idx >= m_ResourceEntries.Count)
                return INVALID_FILE;

            OpenFile.Type = m_ResourceEntries[(int)FileIndex].Entry.ResourceType;
            OpenFile.FileIndex = (int)FileIndex;

            try
            {
                OpenFile.FileStream = m_ResourceEntries[(int)FileIndex].Entry.GetStream(false);
            }
            catch
            {
                return INVALID_FILE;
            }

            //
            // Allocate a handle for it.
            //

            if ((FileHandle = AllocateFileHandle()) == INVALID_FILE)
            {
                m_ResourceEntries[(int)FileIndex].Entry.Release();

                return INVALID_FILE;
            }

            //
            // Insert the new handle table entry.
            //

            try
            {
                m_OpenFileHandles.Add(FileHandle, OpenFile);
            }
            catch
            {
                m_ResourceEntries[(int)FileIndex].Entry.Release();

                return INVALID_FILE;
            }

#if DBGRESOURCE
            System.Diagnostics.Debug.WriteLine("OpenFileByIndex: Opening file " + FileHandle.ToString() + ", filename = " + m_ResourceEntries[(int)FileIndex].Entry.FullName + ", repository = " + m_ResourceEntries[(int)FileIndex].Entry.Repository.Name);
#endif

            //
            // All done, return the new file handle value to the caller.
            //

            return FileHandle;
        }

        //
        // Close an encapsulated file.
        //

        public virtual bool CloseFile(ulong FileHandle)
        {
            ResHandle OpenFile;

            if (!m_OpenFileHandles.TryGetValue(FileHandle, out OpenFile))
                return false;

            m_OpenFileHandles.Remove(FileHandle);

            m_ResourceEntries[OpenFile.FileIndex].Entry.Release();

#if DBGRESOURCE
            System.Diagnostics.Debug.WriteLine("CloseFile: Closed file " + FileHandle.ToString());
#endif

            return true;
        }

        //
        // Read an encapsulated file by handle.  The routine is optimized to
        // operate for sequential file reads.
        //

        public virtual bool ReadEncapsulatedFile(ulong File, IntPtr Offset, IntPtr BytesToRead, out IntPtr BytesRead, byte[] Buffer)
        {
            ResHandle OpenFile;
            long Position;

            BytesRead = IntPtr.Zero;

            if (!m_OpenFileHandles.TryGetValue(File, out OpenFile))
                return false;

            //
            // If we're at the end of file, fail the read.  Otherwise, issue
            // the read request against the stream.
            //

            Position = OpenFile.FileStream.Position;

            if (OpenFile.FileStream.Length == Position)
                return false;

            //
            // If we need to change the file position, do so now.
            //

            if ((long)Offset != Position)
            {
                try
                {
                    OpenFile.FileStream.Seek((long)Offset, SeekOrigin.Begin);
                }
                catch
                {
                    return false;
                }
            }

            //
            // Perform the transfer and call it done.
            //

            BytesRead = (IntPtr)OpenFile.FileStream.Read(Buffer, 0, (int)BytesToRead);

#if DBGRESOURCE
            System.Diagnostics.Debug.WriteLine("ReadEncapsulatedFile: Read " + BytesToRead.ToString() + " bytes (" + BytesToRead.ToString() + " requested) from file " + File.ToString() + " at offset " + Offset.ToString() + ", " + OpenFile.FileStream.Length.ToString() + " bytes total in file.");
#endif

            return true;
        }

        //
        // Return the size of a file.
        //

        public virtual IntPtr GetEncapsulatedFileSize(ulong File)
        {
            ResHandle OpenFile;

            if (!m_OpenFileHandles.TryGetValue(File, out OpenFile))
                return IntPtr.Zero;

#if DBGRESOURCE
            System.Diagnostics.Debug.WriteLine("GetEncapsulatedFileSize: File " + File.ToString() + " length is " + OpenFile.FileStream.Length);
#endif

            return (IntPtr)OpenFile.FileStream.Length;
        }

        //
        // Return the resource type of a file.
        //

        public virtual ushort GetEncapsulatedFileType(ulong File)
        {
            ResHandle OpenFile;

            if (!m_OpenFileHandles.TryGetValue(File, out OpenFile))
                return ResINVALID;

            return OpenFile.Type;
        }

        //
        // Iterate through resources in this resource accessor.  The routine
        // returns false on failure.
        //

        public virtual bool GetEncapsulatedFileEntry(ulong FileIndex, out string ResRef, out ushort ResType)
        {
            int Idx = (int)FileIndex;

            if (Idx < 0 || Idx >= m_ResourceEntries.Count)
            {
                ResRef = "";
                ResType = ResINVALID;

                return false;
            }

            ResRef = m_ResourceEntries[Idx].Entry.ResRef.Value;
            ResType = m_ResourceEntries[Idx].Entry.ResourceType;

            return true;
        }

        //
        // Return the count of encapsulated files in this accessor.
        //

        public virtual ulong GetEncapsulatedFileCount()
        {
            return (ulong)m_ResourceEntries.Count;
        }

        ///////////////////////////////////////////////////////////////////////

        //
        // Define indexing parameters for the resource system.
        //

        //
        // For lookup by file id (i.e. enumeration), an array of ResourceIndex
        // objects, one per resource in the resource system, is maintained.
        //
        // N.B.  There may be duplicates and the list is sorted in canonical
        //       order, with the first element being the most precedent
        //       resource.
        // 

        private struct ResourceIndex
        {
            public int FileIndex;
            public IResourceEntry Entry;
        }

        //
        // For lookup by ResRef/ResType, a dictonary of ResourceKey objects,
        // one per canonical resource, is maintained.  Only the canonical
        // resource for a given ResRef/ResType is present in the dictionary.
        //

        private struct ResourceKey
        {
            public string Name;
            public ushort Type;
        }

        //
        // Define the file handle structure used to store state about an opened
        // file.
        //

        private struct ResHandle
        {
            public Stream FileStream;
            public ushort Type;
            public int FileIndex;
        }

        ///////////////////////////////////////////////////////////////////////

        //
        // Allocate a new file handle.  Note that the returned handle value is
        // only unique of the caller inserts it into the handle table before
        // another call to AllocateFileHandle.
        //

        private ulong AllocateFileHandle()
        {
            ulong Handle;
            ulong FirstHandle;
            ResHandle OpenFile;

            Handle = m_NextFileHandle++;

            if (Handle == INVALID_FILE)
                Handle = m_NextFileHandle++;

            FirstHandle = Handle;

            while (m_OpenFileHandles.TryGetValue(Handle, out OpenFile))
            {
                OpenFile.FileStream = null;

                Handle = m_NextFileHandle++;

                if (Handle == INVALID_FILE)
                    Handle = m_NextFileHandle++;

                //
                // If we've wrapped, then there are no handles left to give
                // out.  Fail.
                //

                if (Handle == FirstHandle)
                    return INVALID_FILE;
            }

            return Handle;
        }

        //
        // Retrieve a canonically ordered array of resource respositories to
        // search.
        //

        private List<IResourceRepository> GetResourceRepositories()
        {
            List<IResourceRepository> Repositories = new List<IResourceRepository>();
            ResourceManager ResMan = ResourceManager.Instance;
            Type [] RepositoryTypeOrdering =
            {
                typeof(ERFResourceRepository),
                typeof(DirectoryResourceRepository),
                typeof(ZIPResourceRepository),
                typeof(KEYResourceRepository)
            };

            //
            // Order the resource repositories in the combined repository list
            // based on their defined hierarchy order.  The repositories that
            // are last added are the most precedent.
            //

            foreach (Type RepoType in RepositoryTypeOrdering)
            {
                foreach (IResourceRepository Repository in ResMan.Repositories)
                {
                    if (RepoType != Repository.GetType())
                        continue;

                    Repositories.Add(Repository);
                }
            }

            return Repositories;
        }

        //
        // Index the OEI resource system into the internal table.
        //

        private void DiscoverResources()
        {
            List<IResourceRepository> Repositories;
            ResourceManager ResMan = ResourceManager.Instance;

            if (m_Plugin != null)
                m_Plugin.OnResourceIndexingStart(this);

            //
            // Create the canonical ordering of resource repositories.
            //

            Repositories = GetResourceRepositories();

            //
            // Index the entire toolset resource system now.
            //
            // The repository list is sorted in most precedent order, that is,
            // the first entry was the most recently added.  However, the
            // resource list is sorted in least precedent order, that is, the
            // first entry was the last one added.
            //

            foreach (IResourceRepository ToolsetRepository in Repositories)
            {
                int MaxId;

                MaxId = ToolsetRepository.Resources.Count;

#if DBGRESOURCE
                PluginUI.AddVerifyResult(
                    NWN2Toolset.Plugins.NWN2VerifyOutputType.Information,
                    "Resource repository: " + ToolsetRepository.Name);
#endif

                //
                // Iterate over each file, creating file entries for each
                // resource type in turn.
                //
                // We search in reverse order, taking the last entry.  This
                // allows us to preserve the order of the most recent entry of
                // a particular tier winning, ensuring that we retrieve the
                // most precedent patched file for inbox datafiles.
                //

                for (int i = MaxId; i != 0; i -= 1)
                {
                    ResourceIndex Index;
                    int ExistingIndex;
                    ResourceKey Key;
                    IResourceEntry ToolsetEntry;

                    ToolsetEntry = ToolsetRepository.Resources[i-1];

                    Index.FileIndex = i-1;
                    Index.Entry = ToolsetEntry;

                    //
                    // Ensure that we have not already claimed this name yet.
                    // We allow only one mapping for a particular name (+type),
                    // and it is the most precedent one in the canonical search
                    // order.
                    //

                    Key.Name = ToolsetEntry.ResRef.Value;
                    Key.Type = ToolsetEntry.ResourceType;

                    //
                    // Skip duplicate entry, we've already found the most
                    // precedent version.
                    //

                    if (m_NameIdMap.TryGetValue(Key, out ExistingIndex))
                        continue;

                    //
                    // First one, add it as the most precedent.
                    //

                    m_ResourceEntries.Add(Index);

                    m_NameIdMap.Add(Key, m_ResourceEntries.Count - 1);
                }
            }

            m_IndexValid = true;

            if (m_Plugin != null)
                m_Plugin.OnResourceIndexingFinish(this);

#if DUMPRESLIST
            using (System.IO.StreamWriter sr = new System.IO.StreamWriter(Path.GetTempPath() + "\\reslist.txt"))
            {
                for (ulong FileId = GetEncapsulatedFileCount(); FileId != 0; FileId -= 1)
                {
                    string ResRef;
                    ushort ResType;

                    if (!GetEncapsulatedFileEntry(FileId - 1, out ResRef, out ResType))
                        sr.WriteLine("Exception getting file entry {0}", FileId - 1);
                    else
                        sr.WriteLine("Entry {0}: Accessor {1}, Name {2}, ResType {3}", FileId - 1, m_ResourceEntries[(int)FileId - 1].Entry.Repository.Name, ResRef, ResType);
                }
            }
#endif
        }

        //
        // Called when a resource element changes and a rescan is necessary.
        //

        private void OnResourceRepositoryInvalidated(OEICollectionWithEvents List)
        {
            //
            // Tell the resource accessor to invalidate its resource indexes.
            //

            InvalidateResourceIndexes();
        }

        private void OnResourceRepositoryAdded(OEICollectionWithEvents List, int Index, object Value)
        {
            IResourceRepository Repository = (IResourceRepository)Value;

            //
            // Subscribe to the new repository.
            //

            Repository.Resources.Changed += this.OnResourceRepositoryContentsInvalidated;

            InvalidateResourceIndexes();
        }

        private void OnResourceRepositoryContentsInvalidated(OEIGenericCollectionWithEvents<IResourceEntry> List)
        {
            //
            // Tell the resource accessor to invalidate its resource indexes.
            //

            InvalidateResourceIndexes();
        }

        ///////////////////////////////////////////////////////////////////////

        //
        // Define the plugin backlink, used for UI indications.
        //

        private NWN2ToolsetCompilerPlugin m_Plugin;

        //
        // Define whether the toolset resource index is still valid, i.e. there
        // haven't been any items added or deleted since.
        //

        private bool m_IndexValid;

        //
        // Define whether index invalidation has been communicated to the
        // target's resource system.
        //

        private bool m_IndexInvalidPending;

        //
        // Define the master file index of the resource system.
        //

        private List<ResourceIndex> m_ResourceEntries;

        //
        // Define the fast name lookup dictionary (resource key mapped to index
        // into m_ResourceEntries).
        //

        private Dictionary<ResourceKey, int> m_NameIdMap;

        //
        // Define the mapping of open file handles to resources (actually,
        // Stream objects).
        //

        private Dictionary<ulong, ResHandle> m_OpenFileHandles;

        //
        // Define the next handle value.
        //

        private ulong m_NextFileHandle;

    }
}
