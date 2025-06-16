#This section is for experimenting with the RAG system 
#Comment in the following section to run the streamlit app in the browser
#note that the vector store is persistent, delete the vector store (rag_index_*) to start fresh


#!/usr/bin/env python3
"""
Multi-Model RAG Application with PDF Support
Supports various embedding models, LLMs, and vector stores
"""

import os
import streamlit as st
import faiss
import numpy as np
from typing import List, Dict, Any, Optional
from dataclasses import dataclass
from pathlib import Path
import pickle
import time

# Core libraries
from sentence_transformers import SentenceTransformer
import PyPDF2
import fitz  # PyMuPDF for better PDF parsing
from langchain.text_splitter import RecursiveCharacterTextSplitter

# LLM integrations
try:
    import openai
    OPENAI_AVAILABLE = True
except ImportError:
    OPENAI_AVAILABLE = False

try:
    import ollama
    OLLAMA_AVAILABLE = True
except ImportError:
    OLLAMA_AVAILABLE = False

@dataclass
class Document:
    content: str
    metadata: Dict[str, Any]

class DocumentProcessor:
    """Handles PDF parsing and text extraction"""

    @staticmethod
    def extract_text_pypdf(pdf_path: str) -> str:
        """Extract text using PyPDF2"""
        text = ""
        with open(pdf_path, 'rb') as file:
            pdf_reader = PyPDF2.PdfReader(file)
            for page in pdf_reader.pages:
                text += page.extract_text() + "\n"
        return text

    @staticmethod
    def extract_text_pymupdf(pdf_path: str) -> str:
        """Extract text using PyMuPDF (better quality)"""
        text = ""
        doc = fitz.open(pdf_path)
        for page_num in range(doc.page_count):
            page = doc[page_num]
            text += page.get_text() + "\n"
        doc.close()
        return text

    @classmethod
    def process_pdf(cls, pdf_path: str, method: str = "pymupdf") -> Document:
        """Process PDF and return Document object"""
        if method == "pymupdf":
            text = cls.extract_text_pymupdf(pdf_path)
        else:
            text = cls.extract_text_pypdf(pdf_path)

        metadata = {
            "source": pdf_path,
            "type": "pdf",
            "processed_at": time.time()
        }

        return Document(content=text, metadata=metadata)

class EmbeddingProvider:
    """Manages different embedding models"""

    SUPPORTED_MODELS = {
        "all-MiniLM-L6-v2": {"dim": 384, "speed": "fast"},
        "all-mpnet-base-v2": {"dim": 768, "speed": "medium"},
        "e5-small-v2": {"dim": 384, "speed": "fast"},
        "e5-large-v2": {"dim": 1024, "speed": "slow"},
        "gte-large": {"dim": 1024, "speed": "medium"}
    }

    def __init__(self, model_name: str):
        self.model_name = model_name
        self.model = SentenceTransformer(f"intfloat/{model_name}" if "e5" in model_name else f"sentence-transformers/{model_name}")
        self.dimension = self.SUPPORTED_MODELS[model_name]["dim"]

    def encode(self, texts: List[str]) -> np.ndarray:
        """Encode texts to embeddings"""
        if "e5" in self.model_name:
            # E5 models expect prefixed queries
            texts = [f"passage: {text}" for text in texts]

        return self.model.encode(texts)

    def encode_query(self, query: str) -> np.ndarray:
        """Encode query (may need different prefix)"""
        if "e5" in self.model_name:
            query = f"query: {query}"

        return self.model.encode([query])[0]

class VectorStore:
    """FAISS-based vector store with persistence"""

    def __init__(self, dimension: int, index_path: Optional[str] = None):
        self.dimension = dimension
        self.index = faiss.IndexFlatIP(dimension)  # Inner product for cosine similarity
        self.documents = []
        self.metadata = []
        self.index_path = index_path

        if index_path and os.path.exists(index_path):
            self.load()

    def add_documents(self, documents: List[Document], embeddings: np.ndarray):
        """Add documents with their embeddings"""
        # Normalize embeddings for cosine similarity
        embeddings = embeddings / np.linalg.norm(embeddings, axis=1, keepdims=True)

        self.index.add(embeddings.astype('float32'))
        self.documents.extend([doc.content for doc in documents])
        self.metadata.extend([doc.metadata for doc in documents])

    def search(self, query_embedding: np.ndarray, k: int = 5) -> List[Dict]:
        """Search for similar documents"""
        # Normalize query embedding
        query_embedding = query_embedding / np.linalg.norm(query_embedding)
        query_embedding = query_embedding.reshape(1, -1).astype('float32')

        scores, indices = self.index.search(query_embedding, k)

        results = []
        for i, (score, idx) in enumerate(zip(scores[0], indices[0])):
            if idx != -1:  # Valid result
                results.append({
                    "content": self.documents[idx],
                    "metadata": self.metadata[idx],
                    "score": float(score),
                    "rank": i + 1
                })

        return results

    def save(self):
        """Save index and metadata"""
        if self.index_path:
            # Save FAISS index
            faiss.write_index(self.index, f"{self.index_path}.faiss")

            # Save metadata
            with open(f"{self.index_path}.pkl", 'wb') as f:
                pickle.dump({
                    "documents": self.documents,
                    "metadata": self.metadata,
                    "dimension": self.dimension
                }, f)

    def load(self):
        """Load index and metadata"""
        if self.index_path:
            # Load FAISS index
            if os.path.exists(f"{self.index_path}.faiss"):
                self.index = faiss.read_index(f"{self.index_path}.faiss")

            # Load metadata
            if os.path.exists(f"{self.index_path}.pkl"):
                with open(f"{self.index_path}.pkl", 'rb') as f:
                    data = pickle.load(f)
                    self.documents = data["documents"]
                    self.metadata = data["metadata"]
                    self.dimension = data["dimension"]

class LLMProvider:
    """Manages different LLM providers"""

    def __init__(self, provider: str, model: str, **kwargs):
        self.provider = provider
        self.model = model
        self.kwargs = kwargs

        if provider == "openai" and OPENAI_AVAILABLE:
            openai.api_key = kwargs.get("api_key", os.getenv("OPENAI_API_KEY"))
        elif provider == "ollama" and not OLLAMA_AVAILABLE:
            st.error("Ollama not available. Install with: pip install ollama")

    def generate(self, prompt: str, context: str) -> str:
        """Generate response using selected LLM"""
        full_prompt = f"""Context: {context}

Question: {prompt}

Please answer the question based on the provided context. If the answer is not in the context, say so.

Answer:"""

        if self.provider == "openai" and OPENAI_AVAILABLE:
            response = openai.ChatCompletion.create(
                model=self.model,
                messages=[{"role": "user", "content": full_prompt}],
                max_tokens=self.kwargs.get("max_tokens", 500),
                temperature=self.kwargs.get("temperature", 0.7)
            )
            return response.choices[0].message.content

        elif self.provider == "ollama" and OLLAMA_AVAILABLE:
            response = ollama.chat(
                model=self.model,
                messages=[{"role": "user", "content": full_prompt}]
            )
            return response['message']['content']

        else:
            return "LLM provider not available or configured."

class RAGSystem:
    """Main RAG system orchestrator"""

    def __init__(self, embedding_model: str, llm_provider: str, llm_model: str, **llm_kwargs):
        self.embedding_provider = EmbeddingProvider(embedding_model)
        self.vector_store = VectorStore(
            dimension=self.embedding_provider.dimension,
            index_path=f"rag_index_{embedding_model}"
        )
        # Only initialize LLM if provider is specified
        if llm_provider is not None:
            self.llm = LLMProvider(llm_provider, llm_model, **llm_kwargs)
        else:
            self.llm = None
        self.text_splitter = RecursiveCharacterTextSplitter(
            chunk_size=1000,
            chunk_overlap=100,
            length_function=len,
        )

    def add_document(self, document: Document):
        """Add a document to the knowledge base"""
        # Split document into chunks
        chunks = self.text_splitter.split_text(document.content)

        # Create chunk documents
        chunk_docs = []
        for i, chunk in enumerate(chunks):
            chunk_metadata = document.metadata.copy()
            chunk_metadata.update({"chunk_id": i, "chunk_count": len(chunks)})
            chunk_docs.append(Document(content=chunk, metadata=chunk_metadata))

        # Generate embeddings
        chunk_texts = [doc.content for doc in chunk_docs]
        embeddings = self.embedding_provider.encode(chunk_texts)

        # Add to vector store
        self.vector_store.add_documents(chunk_docs, embeddings)
        self.vector_store.save()

    def query(self, question: str, k: int = 5) -> Dict:
        """Query the RAG system"""
        # Generate query embedding
        query_embedding = self.embedding_provider.encode_query(question)

        # Search for relevant documents
        search_results = self.vector_store.search(query_embedding, k=k)

        if not search_results:
            return {
                "answer": "No relevant documents found." if self.llm else None,
                "sources": [],
                "search_results": []
            }

        # If no LLM is available, return only search results
        if self.llm is None:
            return {
                "sources": [result['metadata'] for result in search_results],
                "search_results": search_results
            }

        # Prepare context from search results
        context = "\n\n".join([
            f"[Source {i+1}]: {result['content']}"
            for i, result in enumerate(search_results)
        ])

        # Generate answer
        answer = self.llm.generate(question, context)

        return {
            "answer": answer,
            "sources": [result['metadata'] for result in search_results],
            "search_results": search_results
        }

def main():
    """Streamlit UI"""
    st.set_page_config(page_title="Multi-Model RAG System", page_icon="🤖", layout="wide")

    st.title("🤖 Multi-Model RAG System")
    st.markdown("Upload PDFs and query them using various embedding and LLM models")

    # Sidebar for configuration
    with st.sidebar:
        st.header("Configuration")

        # Embedding model selection
        embedding_model = st.selectbox(
            "Embedding Model",
            list(EmbeddingProvider.SUPPORTED_MODELS.keys()),
            help="Choose embedding model (affects speed vs quality)"
        )

        # LLM provider selection
        llm_provider = st.selectbox(
            "LLM Provider",
            ["none", "openai", "ollama"],
            index=0,  # Default to "none"
            help="Choose LLM provider (select 'none' for embedding-only search)"
        )

        if llm_provider == "openai":
            llm_model = st.selectbox("OpenAI Model", ["gpt-3.5-turbo", "gpt-4", "gpt-4-turbo"])
            api_key = st.text_input("OpenAI API Key", type="password", value=os.getenv("OPENAI_API_KEY", ""))
            llm_kwargs = {"api_key": api_key}
        elif llm_provider == "ollama":
            llm_model = st.text_input("Ollama Model", value="llama2", help="Enter Ollama model name")
            llm_kwargs = {}
        else:  # llm_provider == "none"
            llm_model = None
            llm_kwargs = {}

        # Advanced settings
        with st.expander("Advanced Settings"):
            chunk_size = st.slider("Chunk Size", 200, 2000, 1000)
            search_k = st.slider("Search Results (k)", 1, 20, 5)

        # Initialize RAG system button
        if st.button("🚀 Init RAG System", type="primary"):
            try:
                with st.spinner("Initializing RAG system..."):
                    if llm_provider == "none":
                        st.session_state.rag_system = RAGSystem(
                            embedding_model=embedding_model,
                            llm_provider=None,
                            llm_model=None
                        )
                    else:
                        st.session_state.rag_system = RAGSystem(
                            embedding_model=embedding_model,
                            llm_provider=llm_provider,
                            llm_model=llm_model,
                            **llm_kwargs
                        )
                st.success("✅ RAG system initialized successfully!")
            except Exception as e:
                st.error(f"❌ Failed to initialize RAG system: {e}")
                return

    # Check if RAG system is initialized before showing other sections
    if 'rag_system' not in st.session_state:
        st.info("👆 Please initialize the RAG system first using the button in the sidebar.")
        return

    # File upload
    st.header("📄 Document Upload")
    uploaded_files = st.file_uploader(
        "Upload PDF documents",
        type="pdf",
        accept_multiple_files=True
    )

    if uploaded_files:
        for uploaded_file in uploaded_files:
            if st.button(f"Process {uploaded_file.name}"):
                with st.spinner(f"Processing {uploaded_file.name}..."):
                    try:
                        # Save uploaded file temporarily
                        temp_path = f"temp_{uploaded_file.name}"
                        with open(temp_path, "wb") as f:
                            f.write(uploaded_file.getvalue())

                        # Process PDF
                        document = DocumentProcessor.process_pdf(temp_path)
                        st.session_state.rag_system.add_document(document)

                        # Clean up
                        os.remove(temp_path)

                        st.success(f"✅ {uploaded_file.name} processed and added to knowledge base!")

                    except Exception as e:
                        st.error(f"Error processing {uploaded_file.name}: {e}")

    # Query interface
    if llm_provider == "none":
        st.header("🔍 Search Documents")
        st.info("💡 LLM provider is set to 'none' - only similarity search will be performed (no answer generation)")
    else:
        st.header("❓ Query Documents")

    query = st.text_area(
        "Enter your question:" if llm_provider != "none" else "Enter your search query:",
        placeholder="What is the main topic discussed in the documents?" if llm_provider != "none" else "Search for relevant content...",
        height=100
    )

    search_button_text = "🔍 Search" if llm_provider == "none" else "🔍 Search & Answer"

    if st.button(search_button_text, type="primary"):
        if query:
            spinner_text = "Searching documents..." if llm_provider == "none" else "Searching and generating answer..."
            with st.spinner(spinner_text):
                try:
                    result = st.session_state.rag_system.query(query, k=search_k)

                    # Display answer only if LLM is available
                    if llm_provider != "none" and "answer" in result:
                        st.subheader("📝 Answer")
                        st.write(result["answer"])

                    # Display sources
                    if result["search_results"]:
                        st.subheader("📚 Search Results")
                        for i, search_result in enumerate(result["search_results"]):
                            with st.expander(f"Result {i+1} (Score: {search_result['score']:.3f})"):
                                st.write(search_result["content"])
                                st.json(search_result["metadata"])
                    else:
                        st.info("No relevant documents found for your query.")

                except Exception as e:
                    st.error(f"Error during query: {e}")
        else:
            st.warning("Please enter a question." if llm_provider != "none" else "Please enter a search query.")

    # System info
    with st.expander("ℹ️ System Information"):
        if 'rag_system' in st.session_state:
            st.write(f"**Embedding Model:** {embedding_model}")
            st.write(f"**LLM Provider:** {llm_provider}")
            st.write(f"**LLM Model:** {llm_model}")
            st.write(f"**Documents in Knowledge Base:** {len(st.session_state.rag_system.vector_store.documents)}")

if __name__ == "__main__":
    main()
